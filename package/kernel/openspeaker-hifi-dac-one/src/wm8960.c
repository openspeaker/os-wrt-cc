/*
 * wm8960.c  --  WM8960 ALSA SoC Audio driver
 *
 * Copyright 2007-11 Wolfson Microelectronics, plc
 *
 * Author: Liam Girdwood
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <sound/wm8960.h>

#include "wm8960.h"
#include "i2c_wm8960.h"

/* R25 - Power 1 */
#define WM8960_VMID_MASK 0x180
#define WM8960_VREF      0x40

/* R26 - Power 2 */
#define WM8960_PWR2_LOUT1	0x40
#define WM8960_PWR2_ROUT1	0x20
#define WM8960_PWR2_OUT3	0x02

/* R28 - Anti-pop 1 */
#define WM8960_POBCTRL   0x80
#define WM8960_BUFDCOPEN 0x10
#define WM8960_BUFIOEN   0x08
#define WM8960_SOFT_ST   0x04
#define WM8960_HPSTBY    0x01

/* R29 - Anti-pop 2 */
#define WM8960_DISOP     0x40
#define WM8960_DRES_MASK 0x30

static int wm8960_preinit(struct snd_soc_codec *codec)
{
	//printk("****** %s ******\n", __func__);
	snd_soc_write(codec, WM8960_RESET, 0);
	mdelay(500);

	return 0;
}

static int wm8960_postinit(struct snd_soc_codec *codec)
{
	u32 data;
	//printk("****** %s ******\n", __func__);
	// In
	data = snd_soc_read(codec, WM8960_POWER1);
	snd_soc_write(codec, WM8960_POWER1, data|WM8960_PWR1_ADCL|WM8960_PWR1_ADCR|WM8960_PWR1_AINL |WM8960_PWR1_AINR|WM8960_PWR1_MICB);//0x19
	data = snd_soc_read(codec, WM8960_ADDCTL1);
	snd_soc_write(codec, WM8960_ADDCTL1, data|ADDITIONAL1_DATSEL(0x01));//0x17
	snd_soc_write(codec, WM8960_LADC, LEFTGAIN_LDVU|LEFTGAIN_LDACVOL(0xc3));//0x15
	snd_soc_write(codec, WM8960_RADC, LEFTGAIN_LDVU|LEFTGAIN_LDACVOL(0xc3));//0x16
	snd_soc_write(codec, WM8960_LINPATH, 0x148);//0x20
	snd_soc_write(codec, WM8960_RINPATH, 0x148);//0x21
	snd_soc_write(codec, WM8960_POWER3, WM8960_PWR3_LMIC|WM8960_PWR3_RMIC);//0x2f

	// Out
	data = snd_soc_read(codec, WM8960_POWER2);
	snd_soc_write(codec, WM8960_POWER2, data|WM8960_PWR2_DACL|WM8960_PWR2_DACR|WM8960_PWR2_LOUT1|WM8960_PWR2_ROUT1|WM8960_PWR2_SPKL|WM8960_PWR2_SPKR);//0x1a
	mdelay(10);
	snd_soc_write(codec, WM8960_IFACE2, 0x40);
	snd_soc_write(codec, WM8960_LDAC, LEFTGAIN_LDVU|LEFTGAIN_LDACVOL(0xff));//0x0a
	snd_soc_write(codec, WM8960_RDAC, RIGHTGAIN_RDVU|RIGHTGAIN_RDACVOL(0xff));//0x0b
	snd_soc_write(codec, WM8960_LOUTMIX, 0x100);//0x22
	snd_soc_write(codec, WM8960_ROUTMIX, 0x100);//0x25

	data = snd_soc_read(codec, WM8960_POWER3);
	snd_soc_write(codec, WM8960_POWER3, data|WM8960_PWR3_ROMIX|WM8960_PWR3_LOMIX);//0x2f

	snd_soc_write(codec, WM8960_CLASSD1, 0xf7);//0x31
	snd_soc_write(codec, WM8960_CLASSD3, 0xad);//0x33
	snd_soc_write(codec, WM8960_DACCTL1,  0x000);//0x05

	data = snd_soc_read(codec, WM8960_POWER1);
	snd_soc_write(codec, WM8960_POWER1,  data|0x1c0);//0x19


	snd_soc_write(codec, WM8960_LOUT1, LOUT1_LO1VU|LOUT1_LO1ZC|LOUT1_LOUT1VOL(115));//0x02
	snd_soc_write(codec, WM8960_ROUT1, ROUT1_RO1VU|ROUT1_RO1ZC|ROUT1_ROUT1VOL(115));//0x03

	snd_soc_write(codec, WM8960_LINVOL, LINV_IPVU|LINV_LINVOL(110)); //LINV(0x00)=>0x12b
	snd_soc_write(codec, WM8960_RINVOL, RINV_IPVU|RINV_RINVOL(110)); //LINV(0x01)=>0x12b

	return 0;
}

static int wm8960_close(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, WM8960_DACCTL1,0x8); //0x05->0x08
	snd_soc_write(codec, WM8960_POWER1, 0x000); //0x19->0x000
	mdelay(300);
	snd_soc_write(codec, WM8960_POWER2, 0x000); //0x1a->0x000

	return 0;
}

static const DECLARE_TLV_DB_SCALE(dac_tlv, -12700, 50, 1);

static const struct snd_kcontrol_new wm8960_snd_controls[] = {
SOC_DOUBLE_R_TLV("Playback Volume", WM8960_LDAC, WM8960_RDAC,
		 0, 255, 0, dac_tlv),
};

static const struct snd_soc_dapm_widget wm8960_dapm_widgets[] = {
SND_SOC_DAPM_DAC("Left DAC", "Playback", WM8960_POWER2, 8, 0),
SND_SOC_DAPM_DAC("Right DAC", "Playback", WM8960_POWER2, 7, 0),
};

/* Represent OUT3 as a PGA so that it gets turned on with LOUT1/ROUT1 */
static const struct snd_soc_dapm_widget wm8960_dapm_widgets_capless[] = {
SND_SOC_DAPM_PGA("OUT3 VMID", WM8960_POWER2, 1, 0, NULL, 0),
};

static const struct snd_soc_dapm_route audio_paths[] = {
	{ "Left Boost Mixer", "LINPUT1 Switch", "LINPUT1" },
	{ "Left Boost Mixer", "LINPUT2 Switch", "LINPUT2" },
	{ "Left Boost Mixer", "LINPUT3 Switch", "LINPUT3" },

	{ "Left Input Mixer", "Boost Switch", "Left Boost Mixer", },
	{ "Left Input Mixer", NULL, "LINPUT1", },  /* Really Boost Switch */
	{ "Left Input Mixer", NULL, "LINPUT2" },
	{ "Left Input Mixer", NULL, "LINPUT3" },

	{ "Right Boost Mixer", "RINPUT1 Switch", "RINPUT1" },
	{ "Right Boost Mixer", "RINPUT2 Switch", "RINPUT2" },
	{ "Right Boost Mixer", "RINPUT3 Switch", "RINPUT3" },

	{ "Right Input Mixer", "Boost Switch", "Right Boost Mixer", },
	{ "Right Input Mixer", NULL, "RINPUT1", },  /* Really Boost Switch */
	{ "Right Input Mixer", NULL, "RINPUT2" },
	{ "Right Input Mixer", NULL, "RINPUT3" },

	{ "Left ADC", NULL, "Left Input Mixer" },
	{ "Right ADC", NULL, "Right Input Mixer" },

	{ "Left Output Mixer", "LINPUT3 Switch", "LINPUT3" },
	{ "Left Output Mixer", "Boost Bypass Switch", "Left Boost Mixer"} ,
	{ "Left Output Mixer", "PCM Playback Switch", "Left DAC" },

	{ "Right Output Mixer", "RINPUT3 Switch", "RINPUT3" },
	{ "Right Output Mixer", "Boost Bypass Switch", "Right Boost Mixer" } ,
	{ "Right Output Mixer", "PCM Playback Switch", "Right DAC" },

	{ "LOUT1 PGA", NULL, "Left Output Mixer" },
	{ "ROUT1 PGA", NULL, "Right Output Mixer" },

	{ "HP_L", NULL, "LOUT1 PGA" },
	{ "HP_R", NULL, "ROUT1 PGA" },

	{ "Left Speaker PGA", NULL, "Left Output Mixer" },
	{ "Right Speaker PGA", NULL, "Right Output Mixer" },

	{ "Left Speaker Output", NULL, "Left Speaker PGA" },
	{ "Right Speaker Output", NULL, "Right Speaker PGA" },

	{ "SPK_LN", NULL, "Left Speaker Output" },
	{ "SPK_LP", NULL, "Left Speaker Output" },
	{ "SPK_RN", NULL, "Right Speaker Output" },
	{ "SPK_RP", NULL, "Right Speaker Output" },
};

static const struct snd_soc_dapm_route audio_paths_out3[] = {
	{ "Mono Output Mixer", "Left Switch", "Left Output Mixer" },
	{ "Mono Output Mixer", "Right Switch", "Right Output Mixer" },

	{ "OUT3", NULL, "Mono Output Mixer", }
};

static const struct snd_soc_dapm_route audio_paths_capless[] = {
	{ "HP_L", NULL, "OUT3 VMID" },
	{ "HP_R", NULL, "OUT3 VMID" },

	{ "OUT3 VMID", NULL, "Left Output Mixer" },
	{ "OUT3 VMID", NULL, "Right Output Mixer" },
};

static int wm8960_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u16 iface = 0;

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		iface |= 0x0040;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		return -EINVAL;
	}

	/* interface format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		iface |= 0x0002;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		iface |= 0x0001;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		iface |= 0x0003;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		iface |= 0x0013;
		break;
	default:
		return -EINVAL;
	}

	/* clock inversion */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_IB_IF:
		iface |= 0x0090;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		iface |= 0x0080;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		iface |= 0x0010;
		break;
	default:
		return -EINVAL;
	}

	/* set iface */
	snd_soc_write(codec, WM8960_IFACE1, iface);
	wm8960_postinit(codec);
	return 0;
}

static struct {
	int rate;
	unsigned int val;
} alc_rates[] = {
	{ 48000, 0 },
	{ 44100, 0 },
	{ 32000, 1 },
	{ 22050, 2 },
	{ 24000, 2 },
	{ 16000, 3 },
	{ 11025, 4 },
	{ 12000, 4 },
	{  8000, 5 },
};

static int wm8960_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	u16 iface = snd_soc_read(codec, WM8960_IFACE1) & 0xfff3;
	int i;

	/* bit size */
	switch (params_width(params)) {
	case 16:
		break;
	case 20:
		iface |= 0x0004;
		break;
	case 24:
		iface |= 0x0008;
		break;
	default:
		dev_err(codec->dev, "unsupported width %d\n",
			params_width(params));
		return -EINVAL;
	}

	/* Update filters for the new rate */
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
	}

	/* set iface */
	snd_soc_write(codec, WM8960_IFACE1, iface);
	return 0;
}

static int wm8960_mute(struct snd_soc_dai *dai, int mute)
{
	struct snd_soc_codec *codec = dai->codec;

	if (mute)
		snd_soc_update_bits(codec, WM8960_DACCTL1, 0x8, 0x8);
	else
		snd_soc_update_bits(codec, WM8960_DACCTL1, 0x8, 0);
	return 0;
}

/* PLL divisors */
struct _pll_div {
	u32 pre_div:1;
	u32 n:4;
	u32 k:24;
};

/* The size in bits of the pll divide multiplied by 10
 * to allow rounding later */
#define FIXED_PLL_SIZE ((1 << 24) * 10)

static int pll_factors(unsigned int source, unsigned int target,
		       struct _pll_div *pll_div)
{
	unsigned long long Kpart;
	unsigned int K, Ndiv, Nmod;

	pr_debug("WM8960 PLL: setting %dHz->%dHz\n", source, target);

	/* Scale up target to PLL operating frequency */
	target *= 4;

	Ndiv = target / source;
	if (Ndiv < 6) {
		source >>= 1;
		pll_div->pre_div = 1;
		Ndiv = target / source;
	} else
		pll_div->pre_div = 0;

	if ((Ndiv < 6) || (Ndiv > 12)) {
		pr_err("WM8960 PLL: Unsupported N=%d\n", Ndiv);
		return -EINVAL;
	}

	pll_div->n = Ndiv;
	Nmod = target % source;
	Kpart = FIXED_PLL_SIZE * (long long)Nmod;

	do_div(Kpart, source);

	K = Kpart & 0xFFFFFFFF;

	/* Check if we need to round */
	if ((K % 10) >= 5)
		K += 5;

	/* Move down to proper range now rounding is done */
	K /= 10;

	pll_div->k = K;

	pr_debug("WM8960 PLL: N=%x K=%x pre_div=%d\n",
		 pll_div->n, pll_div->k, pll_div->pre_div);

	return 0;
}

static int wm8960_set_dai_pll(struct snd_soc_dai *codec_dai, int pll_id,
		int source, unsigned int freq_in, unsigned int freq_out)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u16 reg;
	static struct _pll_div pll_div;
	int ret;

	if (freq_in && freq_out) {
		ret = pll_factors(freq_in, freq_out, &pll_div);
		if (ret != 0)
			return ret;
	}

	/* Disable the PLL: even if we are changing the frequency the
	 * PLL needs to be disabled while we do so. */
	snd_soc_update_bits(codec, WM8960_CLOCK1, 0x1, 0);
	snd_soc_update_bits(codec, WM8960_POWER2, 0x1, 0);

	if (!freq_in || !freq_out)
		return 0;

	reg = snd_soc_read(codec, WM8960_PLL1) & ~0x3f;
	reg |= pll_div.pre_div << 4;
	reg |= pll_div.n;

	if (pll_div.k) {
		reg |= 0x20;
#if 1
		snd_soc_write(codec, WM8960_PLL2, (pll_div.k >> 16) & 0xff);
		snd_soc_write(codec, WM8960_PLL3, (pll_div.k >> 8) & 0xff);
		snd_soc_write(codec, WM8960_PLL4, pll_div.k & 0xff);
#else
		snd_soc_write(codec, WM8960_PLL2, (pll_div.k >> 16) & 0xff);
		snd_soc_write(codec, WM8960_PLL3, (pll_div.k >> 8) & 0xff);
		snd_soc_write(codec, WM8960_PLL4, pll_div.k & 0xff);
#endif
	}
	snd_soc_write(codec, WM8960_PLL1, reg);

	/* Turn it on */
	snd_soc_update_bits(codec, WM8960_POWER2, 0x1, 0x1);
	msleep(250);
	snd_soc_update_bits(codec, WM8960_CLOCK1, 0x1, 0x1);

	return 0;
}

static int wm8960_set_dai_clkdiv(struct snd_soc_dai *codec_dai,
		int div_id, int div)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u16 reg;

	switch (div_id) {
	case WM8960_SYSCLKDIV:
		reg = snd_soc_read(codec, WM8960_CLOCK1) & 0x1f9;
		snd_soc_write(codec, WM8960_CLOCK1, reg | div);
		break;
	case WM8960_DACDIV:
		reg = snd_soc_read(codec, WM8960_CLOCK1) & 0x1c7;
		snd_soc_write(codec, WM8960_CLOCK1, reg | div);
		break;
	case WM8960_OPCLKDIV:
		reg = snd_soc_read(codec, WM8960_PLL1) & 0x03f;
		snd_soc_write(codec, WM8960_PLL1, reg | div);
		break;
	case WM8960_DCLKDIV:
#if 1
		reg = snd_soc_read(codec, WM8960_CLOCK2) & 0x03f;
#else
		reg = snd_soc_read(codec, WM8960_CLOCK2) & 0x03f;
#endif
		snd_soc_write(codec, WM8960_CLOCK2, reg | div);
		break;
	case WM8960_TOCLKSEL:
		reg = snd_soc_read(codec, WM8960_ADDCTL1) & 0x1fd;
		snd_soc_write(codec, WM8960_ADDCTL1, reg | div);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

#define WM8960_RATES SNDRV_PCM_RATE_8000_48000

#define WM8960_FORMATS \
	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
	SNDRV_PCM_FMTBIT_S24_LE)

static const struct snd_soc_dai_ops wm8960_dai_ops = {
	.hw_params = wm8960_hw_params,
	.digital_mute = wm8960_mute,
	.set_fmt = wm8960_set_dai_fmt,
	.set_clkdiv = wm8960_set_dai_clkdiv,
	.set_pll = wm8960_set_dai_pll,
};

static struct snd_soc_dai_driver wm8960_dai = {
	.name = "wm8960-hifi",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = WM8960_RATES,
		.formats = WM8960_FORMATS,},
/*	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = WM8960_RATES,
		.formats = WM8960_FORMATS,}, */
	.ops = &wm8960_dai_ops,
	.symmetric_rates = 1,
};

static int wm8960_suspend(struct snd_soc_codec *codec)
{
	return 0;
}

static int wm8960_resume(struct snd_soc_codec *codec)
{
	return 0;
}

static int wm8960_probe(struct snd_soc_codec *codec)
{
	struct wm8960_data *pdata = dev_get_platdata(codec->dev);
	int ret = 0;


	snd_soc_add_codec_controls(codec, wm8960_snd_controls,
				     ARRAY_SIZE(wm8960_snd_controls));

	return 0;
}

/* power down chip */
static int wm8960_remove(struct snd_soc_codec *codec)
{
	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_wm8960 = {
	.probe =	wm8960_probe,
	.remove =	wm8960_remove,
	.suspend =	wm8960_suspend,
	.resume =	wm8960_resume,
};

static int wm8960_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct wm8960_data *pdata = dev_get_platdata(&i2c->dev);
	int ret;

	ret = snd_soc_register_codec(&i2c->dev,
			&soc_codec_dev_wm8960, &wm8960_dai, 1);

	return ret;
}

static int wm8960_i2c_remove(struct i2c_client *client)
{
	snd_soc_unregister_codec(&client->dev);
	return 0;
}

static const struct i2c_device_id wm8960_i2c_id[] = {
	{ "wm8960", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, wm8960_i2c_id);

static struct i2c_driver wm8960_i2c_driver = {
	.driver = {
		.name = "wm8960",
		.owner = THIS_MODULE,
	},
	.probe =    wm8960_i2c_probe,
	.remove =   wm8960_i2c_remove,
	.id_table = wm8960_i2c_id,
};

module_i2c_driver(wm8960_i2c_driver);

MODULE_DESCRIPTION("ASoC WM8960 driver");
MODULE_AUTHOR("Liam Girdwood");
MODULE_LICENSE("GPL");
