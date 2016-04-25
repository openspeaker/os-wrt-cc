/*
 * dac-one board codec driver
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


static int wm8960_preinit(struct snd_soc_codec *codec)
{
	return 0;
}

static int wm8960_postinit(struct snd_soc_codec *codec)
{
	return 0;
}

static int wm8960_close(struct snd_soc_codec *codec)
{
	return 0;
}

static const DECLARE_TLV_DB_SCALE(dac_tlv, -12700, 50, 1);

static const struct snd_kcontrol_new wm8960_snd_controls[] = {
//SOC_DOUBLE_R_TLV("Playback Volume", WM8960_LDAC, WM8960_RDAC, 0, 255, 0, dac_tlv),
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
//	snd_soc_write(codec, WM8960_IFACE1, iface);
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
//	u16 iface = snd_soc_read(codec, WM8960_IFACE1) & 0xfff3;
	int i;

	/* bit size */
	switch (params_width(params)) {
	case 16:
		break;
	case 20:
//		iface |= 0x0004;
		break;
	case 24:
//		iface |= 0x0008;
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
//	snd_soc_write(codec, WM8960_IFACE1, iface);
	return 0;
}

static int wm8960_mute(struct snd_soc_dai *dai, int mute)
{
	return 0;
}

/* PLL divisors */
struct _pll_div {
	u32 pre_div:1;
	u32 n:4;
	u32 k:24;
};


static int wm8960_set_dai_pll(struct snd_soc_dai *codec_dai, int pll_id,
		int source, unsigned int freq_in, unsigned int freq_out)
{
	return 0;
}

static int wm8960_set_dai_clkdiv(struct snd_soc_dai *codec_dai,
		int div_id, int div)
{
	return 0;
}

#define WM8960_RATES SNDRV_PCM_RATE_8000_48000

#define WM8960_FORMATS  (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE)

static const struct snd_soc_dai_ops wm8960_dai_ops = {
	.hw_params = wm8960_hw_params,
	.digital_mute = wm8960_mute,
	.set_fmt = wm8960_set_dai_fmt,
	.set_clkdiv = wm8960_set_dai_clkdiv,
	.set_pll = wm8960_set_dai_pll,
};

static struct snd_soc_dai_driver wm8960_dai = {
	.name = "wm8740-dac",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = WM8960_RATES,
		.formats = WM8960_FORMATS,},
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
	{ "wm8740", 0 },
	{ }
};


MODULE_DEVICE_TABLE(i2c, wm8960_i2c_id);

static struct i2c_driver wm8960_i2c_driver = {
       .driver = {
               .name = "wm8740",
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
