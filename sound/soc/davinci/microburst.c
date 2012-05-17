/*
 * Machine driver for EVAL-adau1x61MINIZ on Analog Devices bfin
 * evaluation boards.
 *
 * Copyright 2011 Analog Devices Inc.
 * Author: Lars-Peter Clausen <lars@metafoo.de>
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/module.h>
#include <linux/device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>

#include "../codecs/adau17x1.h"

static const struct snd_soc_dapm_widget bfin_eval_adau1x61_dapm_widgets[] = {
	SND_SOC_DAPM_LINE("In 1", NULL),
	SND_SOC_DAPM_LINE("In 2", NULL),
	SND_SOC_DAPM_LINE("In 3-4", NULL),

	SND_SOC_DAPM_LINE("Diff Out L", NULL),
	SND_SOC_DAPM_LINE("Diff Out R", NULL),
	SND_SOC_DAPM_LINE("Stereo Out", NULL),
	SND_SOC_DAPM_HP("Capless HP Out", NULL),
};

static const struct snd_soc_dapm_route bfin_eval_adau1x61_dapm_routes[] = {
	{ "LAUX", NULL, "In 3-4" },
	{ "RAUX", NULL, "In 3-4" },
	{ "LINP", NULL, "In 1" },
	{ "LINN", NULL, "In 1"},
	{ "RINP", NULL, "In 2" },
	{ "RINN", NULL, "In 2" },

	{ "In 1", NULL, "MICBIAS" },
	{ "In 2", NULL, "MICBIAS" },

	{ "Capless HP Out", NULL, "LHP" },
	{ "Capless HP Out", NULL, "RHP" },
	{ "Diff Out L", NULL, "LOUT" },
	{ "Diff Out R", NULL, "ROUT" },
	{ "Stereo Out", NULL, "LOUT" },
	{ "Stereo Out", NULL, "ROUT" },
};

static int bfin_eval_adau1x61_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	int pll_rate;
	int ret;

	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM);
	if (ret)
		return ret;

	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM);
	if (ret)
		return ret;

	switch (params_rate(params)) {
	case 48000:
	case 8000:
	case 12000:
	case 16000:
	case 24000:
	case 32000:
	case 96000:
		pll_rate = 48000 * 1024;
		break;
	case 44100:
	case 7350:
	case 11025:
	case 14700:
	case 22050:
	case 29400:
	case 88200:
		pll_rate = 44100 * 1024;
		break;
	default:
		return -EINVAL;
	}

	ret = snd_soc_dai_set_pll(codec_dai, ADAU17X1_PLL,
			ADAU17X1_PLL_SRC_MCLK, 12288000, pll_rate);
	if (ret)
		return ret;

	ret = snd_soc_dai_set_sysclk(codec_dai, ADAU17X1_CLK_SRC_PLL, pll_rate,
			SND_SOC_CLOCK_IN);

	return ret;
}

static struct snd_soc_ops bfin_eval_adau1x61_ops = {
	.hw_params = bfin_eval_adau1x61_hw_params,
};

static struct snd_soc_dai_link bfin_eval_adau1x61_dai = {
	.name = "adau1x61",
	.stream_name = "adau1x61",
	.cpu_dai_name = "davinci-mcasp.2", // Is this wrong? bfin used "bfin-i2s.0"
	.codec_dai_name = "adau-hifi", // the codec dai name
	.platform_name = "davinci-pcm-audio", // The platform name that appears to the system
	.codec_name = "adau1761.1-0070", // The codec name to associate with
	.ops = &bfin_eval_adau1x61_ops,
};

static struct snd_soc_card bfin_eval_adau1x61 = {
	.name = "bfin-eval-adau1x61",
	.driver_name = "eval-adau1x61",
	.dai_link = &bfin_eval_adau1x61_dai,
	.num_links = 1,

	.dapm_widgets		= bfin_eval_adau1x61_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(bfin_eval_adau1x61_dapm_widgets),
	.dapm_routes		= bfin_eval_adau1x61_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(bfin_eval_adau1x61_dapm_routes),
};

static int bfin_eval_adau1x61_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &bfin_eval_adau1x61;

	printk("!!!!! bfin_eval_adau1x61_probe:  enter !!!!!\n");
	card->dev = &pdev->dev;

	return snd_soc_register_card(card);
}

static int __devexit bfin_eval_adau1x61_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	printk("!!!!! bfin_eval_adau1x61_remove:  enter !!!!!\n");
	snd_soc_unregister_card(card);

	return 0;
}

static struct platform_driver bfin_eval_adau1x61_driver = {
	.driver = {
		.name = "bfin-eval-adau1x61",
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
	},
	.probe = bfin_eval_adau1x61_probe,
	.remove = __devexit_p(bfin_eval_adau1x61_remove),
};

static int __init bfin_eval_adau1x61_init(void)
{
	printk("!!!!! bfin_eval_adau1x61_init:  enter !!!!!\n");
	return platform_driver_register(&bfin_eval_adau1x61_driver);
}
module_init(bfin_eval_adau1x61_init);

static void __exit bfin_eval_adau1x61_exit(void)
{
	printk("!!!!! bfin_eval_adau1x61_exit:  enter !!!!!\n");
	platform_driver_unregister(&bfin_eval_adau1x61_driver);
}
module_exit(bfin_eval_adau1x61_exit);

MODULE_AUTHOR("Lars-Peter Clausen <lars@metafoo.de>");
MODULE_DESCRIPTION("ALSA SoC bfin adau1x61 driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bfin-eval-adau1x61");
