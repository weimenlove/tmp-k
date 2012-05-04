/*
 * ASoC driver for Microburst
 * adapted from ASoC driver for TI DAVINCI EVM platform
 * by Steve Conklin <steve@conklinhouse.com>
 *
 * Author:      Vladimir Barinov, <vbarinov@embeddedalley.com>
 * Copyright:   (C) 2007 MontaVista Software, Inc., <source@mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/dma.h>
#include <asm/mach-types.h>

#ifndef CONFIG_ARCH_TI81XX
#include <mach/asp.h>
#include <mach/edma.h>
#include <mach/mux.h>
#else
#include <plat/asp.h>
#include <asm/hardware/edma.h>
#endif

#include "../codecs/adau17x1.h"
#include "davinci-pcm.h"
#include "davinci-i2s.h"
#include "davinci-mcasp.h"

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

/* davinci-evm digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link microburst_dai = {
  .name = "adau1x61",
  .stream_name = "adau1x61",
  .cpu_dai_name = "davinci-mcasp.2",
  .codec_dai_name = "adau-hifi",
  .codec_name = "adau1761",
  .platform_name = "davinci-pcm-audio",
  //.init = evm_aic3x_init,
  .ops = &bfin_eval_adau1x61_ops,
};

/* From bfin
static struct snd_soc_card bfin_eval_adau1x61 = {
	.name = "bfin-eval-adau1x61",
	.driver_name = "eval-adau1x61",
	.dai_link = &bfin_eval_adau1x61_dai,
	.num_links = 1,

	.dapm_widgets		= bfin_eval_adau1x61_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(bfin_eval_adau1x61_dapm_widgets),
	.dapm_routes		= bfin_eval_adau1x61_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(bfin_eval_adau1x61_dapm_routes),
	}; */

static struct snd_soc_card microburst_snd_soc_card = {
	.name = "Microburst",
	.dai_link = &microburst_dai,
	.num_links = 1,
	.dapm_widgets		= bfin_eval_adau1x61_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(bfin_eval_adau1x61_dapm_widgets),
	.dapm_routes		= bfin_eval_adau1x61_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(bfin_eval_adau1x61_dapm_routes),
};

static struct platform_device *evm_snd_device;
static int __init evm_init(void)
{
	struct snd_soc_card *evm_snd_dev_data;
	int index;
	int ret;

	printk("*** microburst: Enter evm_init ***\n");
	evm_snd_dev_data = &microburst_snd_soc_card;
	index = 0;


	evm_snd_device = platform_device_alloc("Microburst", index);
	if (!evm_snd_device)
		return -ENOMEM;

	platform_set_drvdata(evm_snd_device, evm_snd_dev_data);
	ret = platform_device_add(evm_snd_device);
	if (ret) {
	        printk("*** microburst: about to put ***\n");
		platform_device_put(evm_snd_device);
	}

	return ret;
}

static void __exit evm_exit(void)
{
	platform_device_unregister(evm_snd_device);
}

module_init(evm_init);
module_exit(evm_exit);

MODULE_AUTHOR("Vladimir Barinov");
MODULE_DESCRIPTION("TI DAVINCI EVM ASoC driver");
MODULE_LICENSE("GPL");
