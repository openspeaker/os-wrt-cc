/*
 * mt76xx_machine.c
 *
 */
#include <linux/init.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#include <linux/sched.h>
#endif

#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
#include <asm/system.h> /* cli(), *_flags */
#endif

#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <sound/core.h>
#include <linux/pci.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <linux/i2c.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include "ralink_gdma.h"
#include "mt76xx_i2s.h"
#include "mt76xx_machine.h"

#if defined(CONFIG_SND_SOC_WM8960)
#include "wm8960.h"
#endif

#define I2C_AUDIO_DEV_ID	(0)
/****************************/
/*FUNCTION DECLRATION		*/
/****************************/
extern unsigned long i2s_codec_12p288Mhz[11];
extern unsigned long i2s_codec_12Mhz[11];


static int mt76xx_codec_clock_hwparams(struct snd_pcm_substream *substream,\
				struct snd_pcm_hw_params *params);
static int mt76xx_codec_startup(struct snd_pcm_substream *substream);
static int mt76xx_codec_init(struct snd_soc_pcm_runtime *rtd);
extern struct snd_soc_dai_driver mt76xx_i2s_dai;
extern struct snd_soc_platform_driver mt76xx_soc_platform;
struct platform_device *mt76xx_audio_device;

#if defined(CONFIG_SND_SOC_WM8960)
extern struct snd_soc_dai wm8960_dai;
extern struct snd_soc_codec_device soc_codec_dev_wm8960;
#endif

static struct snd_soc_ops mtk_audio_ops = {
	.hw_params = mt76xx_codec_clock_hwparams,
	.startup = mt76xx_codec_startup,
};

static struct snd_soc_dai_link mtk_audio_dai = {
	.name = "mtk_dai",
	.stream_name = "WMserious PCM",
	.cpu_dai_name	= "mt76xx-i2s",
	.codec_dai_name	= "wm8741-dac",
	.codec_name	= "wm8741.0-001a",
	.platform_name	= "mt76xx-pcm",
	.ignore_pmdown_time = true,
	.init = mt76xx_codec_init,
	.ops = &mtk_audio_ops,
};

static struct snd_soc_card mtk_audio_card = {
	.name = "MTK APSoC I2S",
	.owner = THIS_MODULE,
	.dai_link = &mtk_audio_dai,//I2S/Codec
	.num_links = 1,
};

static int mt76xx_codec_clock_hwparams(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *p = substream->private_data;
	struct snd_soc_dai *codec_dai = p->codec_dai;
	struct snd_pcm_runtime *runtime = substream->runtime;
	i2s_config_type* rtd = runtime->private_data;
	unsigned long data,index = 0;
	unsigned long* pTable;
	int mclk,ret,targetClk = 0;

	/*For duplex mode, avoid setting twice.*/
	if((rtd->bRxDMAEnable == GDMA_I2S_EN) || (rtd->bTxDMAEnable == GDMA_I2S_EN))
		return 0;
#if defined(CONFIG_I2S_MCLK_12MHZ)
	mclk = 12000000;
#elif defined(CONFIG_I2S_MCLK_12P288MHZ)
	mclk = 12288000;
#else
	mclk = 12000000;
#endif
	//snd_soc_dai_set_sysclk(codec_dai,0,mclk, SND_SOC_CLOCK_IN);

	switch(params_rate(params)){
	case 8000:
		index = 0;
		targetClk = 12288000;
		break;
	case 12000:
		index = 2;
		targetClk = 12288000;
		break;
	case 16000:
		index = 3;
		targetClk = 12288000;
		break;
	case 24000:
		index = 5;
		targetClk = 12288000;
		break;
	case 32000:
		index = 6;
		targetClk = 12288000;
		break;
	case 48000:
		index = 8;
		targetClk = 12288000;
		break;
	case 11025:
		index = 1;
		targetClk = 11289600;
		break;
	case 22050:
		index = 4;
		targetClk = 11289600;
		break;
	case 44100:
		index = 7;
		targetClk = 11289600;
		break;
	case 88200:
		index = 9;
		targetClk = 11289600;
		break;
	case 96000:
		index = 10;
		targetClk = 11289600;
		break;
	default:
		index = 7;
		targetClk = 12288000;
		//MSG("audio sampling rate %u should be %d ~ %d Hz\n", (u32)params_rate(params), MIN_SRATE_HZ, MAX_SRATE_HZ);
		break;
	}
#if defined(CONFIG_SND_SOC_WM8960)
	/*
	 * There is a fixed divide by 4 in the PLL and a selectable
	 * divide by N after the PLL which should be set to divide by 2 to meet this requirement.
	 * */
	ret = snd_soc_dai_set_pll(codec_dai, 0, 0,mclk, targetClk*2);
	/* From app notes: allow Vref to stabilize to reduce clicks */
	if(rtd->slave_en){
		//printk("WM8960 is in master mode\n");
		ret = snd_soc_dai_set_clkdiv(codec_dai, WM8960_DCLKDIV, 0x1c4);
		ret = snd_soc_dai_set_clkdiv(codec_dai, WM8960_SYSCLKDIV, 0x5);
	}

#endif
	if(!rtd->slave_en) {
    printk("ospk:so here for 7866 master mode, codec slave mode\n");
		snd_soc_dai_set_fmt(codec_dai,SND_SOC_DAIFMT_CBS_CFS|SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_NB_NF);
  }
	else{
    printk("ospk:so here for 7866 slave mode, codec master mode\n");
		snd_soc_dai_set_fmt(codec_dai,SND_SOC_DAIFMT_CBM_CFM|SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_NB_NF);
	}
	mdelay(5);

#if defined(CONFIG_SND_SOC_WM8960)
#if defined(CONFIG_I2S_MCLK_12MHZ)
	pTable = i2s_codec_12Mhz;
	data = pTable[index];
#else
	pTable = i2s_codec_12p288Mhz;
	data = pTable[index];
#endif
	if(rtd->codec_pll_en)
		ret = snd_soc_dai_set_clkdiv(codec_dai, WM8960_DACDIV, (data<<3)|0x5);
	else
		ret = snd_soc_dai_set_clkdiv(codec_dai, WM8960_DACDIV, (data<<3|0x4));
#endif

	return 0;
}

static int mt76xx_codec_startup(struct snd_pcm_substream *substream)
{
	//printk("******* %s *******\n", __func__);
	return 0;
}
static int mt76xx_codec_init(struct snd_soc_pcm_runtime *rtd)
{

	//printk("******* %s *******\n", __func__);
	return 0;
}

static struct i2c_board_info i2c_board_info[] = {
	{
#if defined(CONFIG_SND_SOC_WM8750)
		I2C_BOARD_INFO("wm8750", (0x36 >> 1)),
#elif defined(CONFIG_SND_SOC_WM8960)
		I2C_BOARD_INFO("codec_wm8741", (0x34)),
	}, {
		I2C_BOARD_INFO("wm8741", (0x34 >> 1)),
#endif
	}
};

static struct platform_device *soc_mtk_i2s_dev;
static struct platform_device *soc_mtk_pcm_dev;

static int __init mt76xx_machine_init(void)
{
	//struct snd_soc_device *socdev = &mtk_audio_devdata;
	//struct i2c_adapter *adapter = NULL;
	//struct i2c_client *client = NULL;
	int ret = 0;
	struct i2c_adapter *adapter = NULL;
        struct i2c_client *client = NULL;

#if 1

	adapter = i2c_get_adapter(I2C_AUDIO_DEV_ID);
	if (!adapter)
		return -ENODEV;
	client = i2c_new_device(adapter, &i2c_board_info[0]);
	if (!client)
		return -ENODEV;
	i2c_get_clientdata(client);

	client = i2c_new_device(adapter, &i2c_board_info[1]);
	if (!client)
		return -ENODEV;
	i2c_get_clientdata(client);

	i2c_put_adapter(adapter);

#endif

	soc_mtk_i2s_dev =
		platform_device_register_simple("mt76xx-i2s", -1, NULL, 0);
	if (IS_ERR(soc_mtk_i2s_dev))
		return PTR_ERR(soc_mtk_i2s_dev);

	soc_mtk_pcm_dev =
		platform_device_register_simple("mt76xx-pcm", -1, NULL, 0);
	if (IS_ERR(soc_mtk_pcm_dev))
		return PTR_ERR(soc_mtk_pcm_dev);

	mt76xx_audio_device = platform_device_alloc("soc-audio",-1);
	if (mt76xx_audio_device == NULL) {
		ret = -ENOMEM;
		goto err_device_alloc;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
	platform_set_drvdata(mt76xx_audio_device, &mtk_audio_card);
#else
	platform_set_drvdata(mt76xx_audio_device, &mtk_audio_devdata);
	mtk_audio_devdata.dev = &mt76xx_audio_device->dev;
#endif

	/*Ralink I2S register process end*/
	ret = platform_device_add(mt76xx_audio_device);
	if (ret) {
		printk("mtk audio device : platform_device_add failed (%d)\n",ret);
		goto err_device_add;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
#else
	snd_soc_register_dai(&mt76xx_i2s_dai);
#endif

	return 0;

err_device_add:
	if (mt76xx_audio_device!= NULL) {
		platform_device_put(mt76xx_audio_device);
		mt76xx_audio_device = NULL;
	}
err_device_alloc:
	return ret;
}


static void __exit mt76xx_machine_exit(void)
{	

	platform_device_unregister(mt76xx_audio_device);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,20)
	/* Do nothing */
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
	snd_soc_unregister_platform(&mt76xx_audio_device->dev);
#else
	snd_soc_unregister_platform(&mt76xx_soc_platform);
#endif
	platform_device_unregister(soc_mtk_i2s_dev);
	platform_device_unregister(soc_mtk_pcm_dev);

	mt76xx_audio_device = NULL;
}

//module_init(mt76xx_machine_init);
late_initcall(mt76xx_machine_init);
module_exit(mt76xx_machine_exit);
//EXPORT_SYMBOL_GPL(mt76xx_soc_platform);
MODULE_LICENSE("GPL");
