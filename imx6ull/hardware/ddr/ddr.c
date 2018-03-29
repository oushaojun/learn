/*************
���ܣ�LDDR
MYiR-iMX-uboot\arch\arm\cpu\armv7\mx6\ddr.c

#include <common.h>
#include <linux/types.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/types.h>

**************/ 

#define MX6_IOM_DRAM_DQM0	0x020e0244
#define MX6_IOM_DRAM_DQM1	0x020e0248

#define MX6_IOM_DRAM_RAS	0x020e024c
#define MX6_IOM_DRAM_CAS	0x020e0250
#define MX6_IOM_DRAM_CS0	0x020e0254
#define MX6_IOM_DRAM_CS1	0x020e0258
#define MX6_IOM_DRAM_SDWE_B	0x020e025c
#define MX6_IOM_DRAM_SDODT0	0x020e0260
#define MX6_IOM_DRAM_SDODT1	0x020e0264
#define MX6_IOM_DRAM_SDBA0	0x020e0268
#define MX6_IOM_DRAM_SDBA1	0x020e026c
#define MX6_IOM_DRAM_SDBA2	0x020e0270
#define MX6_IOM_DRAM_SDCKE0	0x020e0274
#define MX6_IOM_DRAM_SDCKE1	0x020e0278
#define MX6_IOM_DRAM_SDCLK_0	0x020e027c
#define MX6_IOM_DRAM_SDQS0	0x020e0280
#define MX6_IOM_DRAM_SDQS1	0x020e0284
#define MX6_IOM_DRAM_RESET	0x020e0288

#define MX6_IOM_GRP_ADDDS	0x020e0490
#define MX6_IOM_DDRMODE_CTL	0x020e0494
#define MX6_IOM_GRP_B0DS	0x020e0498
#define MX6_IOM_GRP_DDRPK	0x020e049c
#define MX6_IOM_GRP_CTLDS	0x020e04a0
#define MX6_IOM_GRP_B1DS	0x020e04a4
#define MX6_IOM_GRP_DDRHYS	0x020e04a8
#define MX6_IOM_GRP_DDRPKE	0x020e04ac
#define MX6_IOM_GRP_DDRMODE	0x020e04b0
#define MX6_IOM_GRP_DDR_TYPE	0x020e04b4

#if 0

void mx6ul_dram_iocfg(unsigned width,
		      const struct mx6ul_iomux_ddr_regs *ddr,
		      const struct mx6ul_iomux_grp_regs *grp)
{
	struct mx6ul_iomux_ddr_regs *mx6_ddr_iomux;
	struct mx6ul_iomux_grp_regs *mx6_grp_iomux;

	mx6_ddr_iomux = (struct mx6ul_iomux_ddr_regs *)MX6UL_IOM_DDR_BASE;
	mx6_grp_iomux = (struct mx6ul_iomux_grp_regs *)MX6UL_IOM_GRP_BASE;

	/* DDR IO TYPE */
	writel(grp->grp_ddr_type, &mx6_grp_iomux->grp_ddr_type);
	writel(grp->grp_ddrpke, &mx6_grp_iomux->grp_ddrpke);

	/* CLOCK */
	writel(ddr->dram_sdclk_0, &mx6_ddr_iomux->dram_sdclk_0);

	/* ADDRESS */
	writel(ddr->dram_cas, &mx6_ddr_iomux->dram_cas);
	writel(ddr->dram_ras, &mx6_ddr_iomux->dram_ras);
	writel(grp->grp_addds, &mx6_grp_iomux->grp_addds);

	/* Control */
	writel(ddr->dram_reset, &mx6_ddr_iomux->dram_reset);
	writel(ddr->dram_sdba2, &mx6_ddr_iomux->dram_sdba2);
	writel(ddr->dram_odt0, &mx6_ddr_iomux->dram_odt0);
	writel(ddr->dram_odt1, &mx6_ddr_iomux->dram_odt1);
	writel(grp->grp_ctlds, &mx6_grp_iomux->grp_ctlds);

	/* Data Strobes */
	writel(grp->grp_ddrmode_ctl, &mx6_grp_iomux->grp_ddrmode_ctl);
	writel(ddr->dram_sdqs0, &mx6_ddr_iomux->dram_sdqs0);
	writel(ddr->dram_sdqs1, &mx6_ddr_iomux->dram_sdqs1);

	/* Data */
	writel(grp->grp_ddrmode, &mx6_grp_iomux->grp_ddrmode);
	writel(grp->grp_b0ds, &mx6_grp_iomux->grp_b0ds);
	writel(grp->grp_b1ds, &mx6_grp_iomux->grp_b1ds);
	writel(ddr->dram_dqm0, &mx6_ddr_iomux->dram_dqm0);
	writel(ddr->dram_dqm1, &mx6_ddr_iomux->dram_dqm1);
}




/*
 * Configure mx6 mmdc registers based on:
 *  - board-specific memory configuration
 *  - board-specific calibration data
 *  - ddr3/lpddr2 chip details
 *
 * The various calculations here are derived from the Freescale
 * 1. i.Mx6DQSDL DDR3 Script Aid spreadsheet (DOC-94917) designed to generate
 *    MMDC configuration registers based on memory system and memory chip
 *    parameters.
 *
 * 2. i.Mx6SL LPDDR2 Script Aid spreadsheet V0.04 designed to generate MMDC
 *    configuration registers based on memory system and memory chip
 *    parameters.
 *
 * The defaults here are those which were specified in the spreadsheet.
 * For details on each register, refer to the IMX6DQRM and/or IMX6SDLRM
 * and/or IMX6SLRM section titled MMDC initialization.
 */
#define MR(val, ba, cmd, cs1) \
	((val << 16) | (1 << 15) | (cmd << 4) | (cs1 << 3) | ba)
#define MMDC1(entry, value) do {					  \
	if (!is_cpu_type(MXC_CPU_MX6SX) && !is_cpu_type(MXC_CPU_MX6UL) && \
	    !is_cpu_type(MXC_CPU_MX6SL))				  \
		mmdc1->entry = value;					  \
	} while (0)

/*
 * According JESD209-2B-LPDDR2: Table 103
 * WL: write latency
 */
static int lpddr2_wl(uint32_t mem_speed)
{
	switch (mem_speed) {
	case 1066:
	case 933:
		return 4;
	case 800:
		return 3;
	case 677:
	case 533:
		return 2;
	case 400:
	case 333:
		return 1;
	default:
		puts("invalid memory speed\n");
		hang();
	}

	return 0;
}

/*
 * According JESD209-2B-LPDDR2: Table 103
 * RL: read latency
 */
static int lpddr2_rl(uint32_t mem_speed)
{
	switch (mem_speed) {
	case 1066:
		return 8;
	case 933:
		return 7;
	case 800:
		return 6;
	case 677:
		return 5;
	case 533:
		return 4;
	case 400:
	case 333:
		return 3;
	default:
		puts("invalid memory speed\n");
		hang();
	}

	return 0;
}

void mx6_lpddr2_cfg(const struct mx6_ddr_sysinfo *sysinfo,
		    const struct mx6_mmdc_calibration *calib,
		    const struct mx6_lpddr2_cfg *lpddr2_cfg)
{
	volatile struct mmdc_p_regs *mmdc0;
	u32 val;
	u8 tcke, tcksrx, tcksre, trrd;
	u8 twl, txp, tfaw, tcl;
	u16 tras, twr, tmrd, trtp, twtr, trfc, txsr;
	u16 trcd_lp, trppb_lp, trpab_lp, trc_lp;
	u16 cs0_end;
	u8 coladdr;
	int clkper; /* clock period in picoseconds */
	int clock;  /* clock freq in mHz */
	int cs;

	/* only support 16/32 bits */
	if (sysinfo->dsize > 1)
		hang();

	mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;

	clock = mxc_get_clock(MXC_DDR_CLK) / 1000000U;
	clkper = (1000 * 1000) / clock; /* pico seconds */

	twl = lpddr2_wl(lpddr2_cfg->mem_speed) - 1;

	/* LPDDR2-S2 and LPDDR2-S4 have the same tRFC value. */
	switch (lpddr2_cfg->density) {
	case 1:
	case 2:
	case 4:
		trfc = DIV_ROUND_UP(130000, clkper) - 1;
		txsr = DIV_ROUND_UP(140000, clkper) - 1;
		break;
	case 8:
		trfc = DIV_ROUND_UP(210000, clkper) - 1;
		txsr = DIV_ROUND_UP(220000, clkper) - 1;
		break;
	default:
		/*
		 * 64Mb, 128Mb, 256Mb, 512Mb are not supported currently.
		 */
		hang();
		break;
	}
	/*
	 * txpdll, txpr, taonpd and taofpd are not relevant in LPDDR2 mode,
	 * set them to 0. */
	txp = DIV_ROUND_UP(7500, clkper) - 1;
	tcke = 3;
	if (lpddr2_cfg->mem_speed == 333)
		tfaw = DIV_ROUND_UP(60000, clkper) - 1;
	else
		tfaw = DIV_ROUND_UP(50000, clkper) - 1;
	trrd = DIV_ROUND_UP(10000, clkper) - 1;

	/* tckesr for LPDDR2 */
	tcksre = DIV_ROUND_UP(15000, clkper);
	tcksrx = tcksre;
	twr  = DIV_ROUND_UP(15000, clkper) - 1;
	/*
	 * tMRR: 2, tMRW: 5
	 * tMRD should be set to max(tMRR, tMRW)
	 */
	tmrd = 5;
	tras = DIV_ROUND_UP(lpddr2_cfg->trasmin, clkper / 10) - 1;
	/* LPDDR2 mode use tRCD_LP filed in MDCFG3. */
	trcd_lp = DIV_ROUND_UP(lpddr2_cfg->trcd_lp, clkper / 10) - 1;
	trc_lp = DIV_ROUND_UP(lpddr2_cfg->trasmin + lpddr2_cfg->trppb_lp,
			      clkper / 10) - 1;
	trppb_lp = DIV_ROUND_UP(lpddr2_cfg->trppb_lp, clkper / 10) - 1;
	trpab_lp = DIV_ROUND_UP(lpddr2_cfg->trpab_lp, clkper / 10) - 1;
	/* To LPDDR2, CL in MDCFG0 refers to RL */
	tcl = lpddr2_rl(lpddr2_cfg->mem_speed) - 3;
	twtr = DIV_ROUND_UP(7500, clkper) - 1;
	trtp = DIV_ROUND_UP(7500, clkper) - 1;

	cs0_end = 4 * sysinfo->cs_density - 1;

	debug("density:%d Gb (%d Gb per chip)\n",
	      sysinfo->cs_density, lpddr2_cfg->density);
	debug("clock: %dMHz (%d ps)\n", clock, clkper);
	debug("memspd:%d\n", lpddr2_cfg->mem_speed);
	debug("trcd_lp=%d\n", trcd_lp);
	debug("trppb_lp=%d\n", trppb_lp);
	debug("trpab_lp=%d\n", trpab_lp);
	debug("trc_lp=%d\n", trc_lp);
	debug("tcke=%d\n", tcke);
	debug("tcksrx=%d\n", tcksrx);
	debug("tcksre=%d\n", tcksre);
	debug("trfc=%d\n", trfc);
	debug("txsr=%d\n", txsr);
	debug("txp=%d\n", txp);
	debug("tfaw=%d\n", tfaw);
	debug("tcl=%d\n", tcl);
	debug("tras=%d\n", tras);
	debug("twr=%d\n", twr);
	debug("tmrd=%d\n", tmrd);
	debug("twl=%d\n", twl);
	debug("trtp=%d\n", trtp);
	debug("twtr=%d\n", twtr);
	debug("trrd=%d\n", trrd);
	debug("cs0_end=%d\n", cs0_end);
	debug("ncs=%d\n", sysinfo->ncs);

	/*
	 * board-specific configuration:
	 *  These values are determined empirically and vary per board layout
	 */
	mmdc0->mpwldectrl0 = calib->p0_mpwldectrl0;
	mmdc0->mpwldectrl1 = calib->p0_mpwldectrl1;
	mmdc0->mpdgctrl0 = calib->p0_mpdgctrl0;
	mmdc0->mpdgctrl1 = calib->p0_mpdgctrl1;
	mmdc0->mprddlctl = calib->p0_mprddlctl;
	mmdc0->mpwrdlctl = calib->p0_mpwrdlctl;
	mmdc0->mpzqlp2ctl = calib->mpzqlp2ctl;

	/* Read data DQ Byte0-3 delay */
	mmdc0->mprddqby0dl = 0x33333333;
	mmdc0->mprddqby1dl = 0x33333333;
	if (sysinfo->dsize > 0) {
		mmdc0->mprddqby2dl = 0x33333333;
		mmdc0->mprddqby3dl = 0x33333333;
	}

	/* Write data DQ Byte0-3 delay */
	mmdc0->mpwrdqby0dl = 0xf3333333;
	mmdc0->mpwrdqby1dl = 0xf3333333;
	if (sysinfo->dsize > 0) {
		mmdc0->mpwrdqby2dl = 0xf3333333;
		mmdc0->mpwrdqby3dl = 0xf3333333;
	}

	/*
	 * In LPDDR2 mode this register should be cleared,
	 * so no termination will be activated.
	 */
	mmdc0->mpodtctrl = 0;

	/* complete calibration */
	val = (1 << 11); /* Force measurement on delay-lines */
	mmdc0->mpmur0 = val;

	/* Step 1: configuration request */
	mmdc0->mdscr = (u32)(1 << 15); /* config request */

	/* Step 2: Timing configuration */
	mmdc0->mdcfg0 = (trfc << 24) | (txsr << 16) | (txp << 13) |
			(tfaw << 4) | tcl;
	mmdc0->mdcfg1 = (tras << 16) | (twr << 9) | (tmrd << 5) | twl;
	mmdc0->mdcfg2 = (trtp << 6) | (twtr << 3) | trrd;
	mmdc0->mdcfg3lp = (trc_lp << 16) | (trcd_lp << 8) |
			  (trppb_lp << 4) | trpab_lp;
	mmdc0->mdotc = 0;

	mmdc0->mdasp = cs0_end; /* CS addressing */

	/* Step 3: Configure DDR type */
	mmdc0->mdmisc = (sysinfo->cs1_mirror << 19) | (sysinfo->walat << 16) |
			(sysinfo->bi_on << 12) | (sysinfo->mif3_mode << 9) |
			(sysinfo->ralat << 6) | (1 << 3);

	/* Step 4: Configure delay while leaving reset */
	mmdc0->mdor = (sysinfo->sde_to_rst << 8) |
		      (sysinfo->rst_to_cke << 0);

	/* Step 5: Configure DDR physical parameters (density and burst len) */
	coladdr = lpddr2_cfg->coladdr;
	if (lpddr2_cfg->coladdr == 8)		/* 8-bit COL is 0x3 */
		coladdr += 4;
	else if (lpddr2_cfg->coladdr == 12)	/* 12-bit COL is 0x4 */
		coladdr += 1;
	mmdc0->mdctl =  (lpddr2_cfg->rowaddr - 11) << 24 |	/* ROW */
			(coladdr - 9) << 20 |			/* COL */
			(0 << 19) |	/* Burst Length = 4 for LPDDR2 */
			(sysinfo->dsize << 16);	/* DDR data bus size */

	/* Step 6: Perform ZQ calibration */
	val = 0xa1390003; /* one-time HW ZQ calib */
	mmdc0->mpzqhwctrl = val;

	/* Step 7: Enable MMDC with desired chip select */
	mmdc0->mdctl |= (1 << 31) |			     /* SDE_0 for CS0 */
			((sysinfo->ncs == 2) ? 1 : 0) << 30; /* SDE_1 for CS1 */

	/* Step 8: Write Mode Registers to Init LPDDR2 devices */
	for (cs = 0; cs < sysinfo->ncs; cs++) {
		/* MR63: reset */
		mmdc0->mdscr = MR(63, 0, 3, cs);
		/* MR10: calibration,
		 * 0xff is calibration command after intilization.
		 */
		val = 0xA | (0xff << 8);
		mmdc0->mdscr = MR(val, 0, 3, cs);
		/* MR1 */
		val = 0x1 | (0x82 << 8);
		mmdc0->mdscr = MR(val, 0, 3, cs);
		/* MR2 */
		val = 0x2 | (0x04 << 8);
		mmdc0->mdscr = MR(val, 0, 3, cs);
		/* MR3 */
		val = 0x3 | (0x02 << 8);
		mmdc0->mdscr = MR(val, 0, 3, cs);
	}

	/* Step 10: Power down control and self-refresh */
	mmdc0->mdpdc = (tcke & 0x7) << 16 |
			5            << 12 |  /* PWDT_1: 256 cycles */
			5            <<  8 |  /* PWDT_0: 256 cycles */
			1            <<  6 |  /* BOTH_CS_PD */
			(tcksrx & 0x7) << 3 |
			(tcksre & 0x7);
	mmdc0->mapsr = 0x00001006; /* ADOPT power down enabled */

	/* Step 11: Configure ZQ calibration: one-time and periodic 1ms */
	val = 0xa1310003;
	mmdc0->mpzqhwctrl = val;

	/* Step 12: Configure and activate periodic refresh */
	mmdc0->mdref = (0 << 14) | /* REF_SEL: Periodic refresh cycle: 64kHz */
		       (3 << 11);  /* REFR: Refresh Rate - 4 refreshes */

	/* Step 13: Deassert config request - init complete */
	mmdc0->mdscr = 0x00000000;

	/* wait for auto-ZQ calibration to complete */
	mdelay(1);
}

void mx6_ddr3_cfg(const struct mx6_ddr_sysinfo *sysinfo,
		  const struct mx6_mmdc_calibration *calib,
		  const struct mx6_ddr3_cfg *ddr3_cfg)
{
	volatile struct mmdc_p_regs *mmdc0;
	volatile struct mmdc_p_regs *mmdc1;
	u32 val;
	u8 tcke, tcksrx, tcksre, txpdll, taofpd, taonpd, trrd;
	u8 todtlon, taxpd, tanpd, tcwl, txp, tfaw, tcl;
	u8 todt_idle_off = 0x4; /* from DDR3 Script Aid spreadsheet */
	u16 trcd, trc, tras, twr, tmrd, trtp, trp, twtr, trfc, txs, txpr;
	u16 cs0_end;
	u16 tdllk = 0x1ff; /* DLL locking time: 512 cycles (JEDEC DDR3) */
	u8 coladdr;
	int clkper; /* clock period in picoseconds */
	int clock; /* clock freq in MHz */
	int cs;
	u16 mem_speed = ddr3_cfg->mem_speed;

	mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;
	if (!is_cpu_type(MXC_CPU_MX6SX) && !is_cpu_type(MXC_CPU_MX6UL) &&
	    !is_cpu_type(MXC_CPU_MX6SL))
		mmdc1 = (struct mmdc_p_regs *)MMDC_P1_BASE_ADDR;

	/* Limit mem_speed for MX6D/MX6Q */
	if (is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D) ||
		is_cpu_type(MXC_CPU_MX6QP) || is_cpu_type(MXC_CPU_MX6DP)) {
		if (mem_speed > 1066)
			mem_speed = 1066; /* 1066 MT/s */

		tcwl = 4;
	}
	/* Limit mem_speed for MX6S/MX6DL */
	else {
		if (mem_speed > 800)
			mem_speed = 800;  /* 800 MT/s */

		tcwl = 3;
	}

	clock = mem_speed / 2;
	/*
	 * Data rate of 1066 MT/s requires 533 MHz DDR3 clock, but MX6D/Q supports
	 * up to 528 MHz, so reduce the clock to fit chip specs
	 */
	if (is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D) ||
		is_cpu_type(MXC_CPU_MX6QP) || is_cpu_type(MXC_CPU_MX6DP)) {
		if (clock > 528)
			clock = 528; /* 528 MHz */
	}

	clkper = (1000 * 1000) / clock; /* pico seconds */
	todtlon = tcwl;
	taxpd = tcwl;
	tanpd = tcwl;

	switch (ddr3_cfg->density) {
	case 1: /* 1Gb per chip */
		trfc = DIV_ROUND_UP(110000, clkper) - 1;
		txs = DIV_ROUND_UP(120000, clkper) - 1;
		break;
	case 2: /* 2Gb per chip */
		trfc = DIV_ROUND_UP(160000, clkper) - 1;
		txs = DIV_ROUND_UP(170000, clkper) - 1;
		break;
	case 4: /* 4Gb per chip */
		trfc = DIV_ROUND_UP(260000, clkper) - 1;
		txs = DIV_ROUND_UP(270000, clkper) - 1;
		break;
	case 8: /* 8Gb per chip */
		trfc = DIV_ROUND_UP(350000, clkper) - 1;
		txs = DIV_ROUND_UP(360000, clkper) - 1;
		break;
	default:
		/* invalid density */
		puts("invalid chip density\n");
		hang();
		break;
	}
	txpr = txs;

	switch (mem_speed) {
	case 800:
		txp = DIV_ROUND_UP(max(3 * clkper, 7500), clkper) - 1;
		tcke = DIV_ROUND_UP(max(3 * clkper, 7500), clkper) - 1;
		if (ddr3_cfg->pagesz == 1) {
			tfaw = DIV_ROUND_UP(40000, clkper) - 1;
			trrd = DIV_ROUND_UP(max(4 * clkper, 10000), clkper) - 1;
		} else {
			tfaw = DIV_ROUND_UP(50000, clkper) - 1;
			trrd = DIV_ROUND_UP(max(4 * clkper, 10000), clkper) - 1;
		}
		break;
	case 1066:
		txp = DIV_ROUND_UP(max(3 * clkper, 7500), clkper) - 1;
		tcke = DIV_ROUND_UP(max(3 * clkper, 5625), clkper) - 1;
		if (ddr3_cfg->pagesz == 1) {
			tfaw = DIV_ROUND_UP(37500, clkper) - 1;
			trrd = DIV_ROUND_UP(max(4 * clkper, 7500), clkper) - 1;
		} else {
			tfaw = DIV_ROUND_UP(50000, clkper) - 1;
			trrd = DIV_ROUND_UP(max(4 * clkper, 10000), clkper) - 1;
		}
		break;
	default:
		puts("invalid memory speed\n");
		hang();
		break;
	}
	txpdll = DIV_ROUND_UP(max(10 * clkper, 24000), clkper) - 1;
	tcksre = DIV_ROUND_UP(max(5 * clkper, 10000), clkper);
	taonpd = DIV_ROUND_UP(2000, clkper) - 1;
	tcksrx = tcksre;
	taofpd = taonpd;
	twr  = DIV_ROUND_UP(15000, clkper) - 1;
	tmrd = DIV_ROUND_UP(max(12 * clkper, 15000), clkper) - 1;
	trc  = DIV_ROUND_UP(ddr3_cfg->trcmin, clkper / 10) - 1;
	tras = DIV_ROUND_UP(ddr3_cfg->trasmin, clkper / 10) - 1;
	tcl  = DIV_ROUND_UP(ddr3_cfg->trcd, clkper / 10) - 3;
	trp  = DIV_ROUND_UP(ddr3_cfg->trcd, clkper / 10) - 1;
	twtr = ROUND(max(4 * clkper, 7500) / clkper, 1) - 1;
	trcd = trp;
	trtp = twtr;
	cs0_end = 4 * sysinfo->cs_density - 1;

	debug("density:%d Gb (%d Gb per chip)\n",
	      sysinfo->cs_density, ddr3_cfg->density);
	debug("clock: %dMHz (%d ps)\n", clock, clkper);
	debug("memspd:%d\n", mem_speed);
	debug("tcke=%d\n", tcke);
	debug("tcksrx=%d\n", tcksrx);
	debug("tcksre=%d\n", tcksre);
	debug("taofpd=%d\n", taofpd);
	debug("taonpd=%d\n", taonpd);
	debug("todtlon=%d\n", todtlon);
	debug("tanpd=%d\n", tanpd);
	debug("taxpd=%d\n", taxpd);
	debug("trfc=%d\n", trfc);
	debug("txs=%d\n", txs);
	debug("txp=%d\n", txp);
	debug("txpdll=%d\n", txpdll);
	debug("tfaw=%d\n", tfaw);
	debug("tcl=%d\n", tcl);
	debug("trcd=%d\n", trcd);
	debug("trp=%d\n", trp);
	debug("trc=%d\n", trc);
	debug("tras=%d\n", tras);
	debug("twr=%d\n", twr);
	debug("tmrd=%d\n", tmrd);
	debug("tcwl=%d\n", tcwl);
	debug("tdllk=%d\n", tdllk);
	debug("trtp=%d\n", trtp);
	debug("twtr=%d\n", twtr);
	debug("trrd=%d\n", trrd);
	debug("txpr=%d\n", txpr);
	debug("cs0_end=%d\n", cs0_end);
	debug("ncs=%d\n", sysinfo->ncs);
	debug("Rtt_wr=%d\n", sysinfo->rtt_wr);
	debug("Rtt_nom=%d\n", sysinfo->rtt_nom);
	debug("SRT=%d\n", ddr3_cfg->SRT);
	debug("twr=%d\n", twr);

	/*
	 * board-specific configuration:
	 *  These values are determined empirically and vary per board layout
	 *  see:
	 *   appnote, ddr3 spreadsheet
	 */
	mmdc0->mpwldectrl0 = calib->p0_mpwldectrl0;
	mmdc0->mpwldectrl1 = calib->p0_mpwldectrl1;
	mmdc0->mpdgctrl0 = calib->p0_mpdgctrl0;
	mmdc0->mpdgctrl1 = calib->p0_mpdgctrl1;
	mmdc0->mprddlctl = calib->p0_mprddlctl;
	mmdc0->mpwrdlctl = calib->p0_mpwrdlctl;
	if (sysinfo->dsize > 1) {
		MMDC1(mpwldectrl0, calib->p1_mpwldectrl0);
		MMDC1(mpwldectrl1, calib->p1_mpwldectrl1);
		MMDC1(mpdgctrl0, calib->p1_mpdgctrl0);
		MMDC1(mpdgctrl1, calib->p1_mpdgctrl1);
		MMDC1(mprddlctl, calib->p1_mprddlctl);
		MMDC1(mpwrdlctl, calib->p1_mpwrdlctl);
	}

	/* Read data DQ Byte0-3 delay */
	mmdc0->mprddqby0dl = 0x33333333;
	mmdc0->mprddqby1dl = 0x33333333;
	if (sysinfo->dsize > 0) {
		mmdc0->mprddqby2dl = 0x33333333;
		mmdc0->mprddqby3dl = 0x33333333;
	}

	if (sysinfo->dsize > 1) {
		MMDC1(mprddqby0dl, 0x33333333);
		MMDC1(mprddqby1dl, 0x33333333);
		MMDC1(mprddqby2dl, 0x33333333);
		MMDC1(mprddqby3dl, 0x33333333);
	}

	/* MMDC Termination: rtt_nom:2 RZQ/2(120ohm), rtt_nom:1 RZQ/4(60ohm) */
	val = (sysinfo->rtt_nom == 2) ? 0x00011117 : 0x00022227;
	mmdc0->mpodtctrl = val;
	if (sysinfo->dsize > 1)
		MMDC1(mpodtctrl, val);

	/* complete calibration */
	val = (1 << 11); /* Force measurement on delay-lines */
	mmdc0->mpmur0 = val;
	if (sysinfo->dsize > 1)
		MMDC1(mpmur0, val);

	/* Step 1: configuration request */
	mmdc0->mdscr = (u32)(1 << 15); /* config request */

	/* Step 2: Timing configuration */
	mmdc0->mdcfg0 = (trfc << 24) | (txs << 16) | (txp << 13) |
			(txpdll << 9) | (tfaw << 4) | tcl;
	mmdc0->mdcfg1 = (trcd << 29) | (trp << 26) | (trc << 21) |
			(tras << 16) | (1 << 15) /* trpa */ |
			(twr << 9) | (tmrd << 5) | tcwl;
	mmdc0->mdcfg2 = (tdllk << 16) | (trtp << 6) | (twtr << 3) | trrd;
	mmdc0->mdotc = (taofpd << 27) | (taonpd << 24) | (tanpd << 20) |
		       (taxpd << 16) | (todtlon << 12) | (todt_idle_off << 4);
	mmdc0->mdasp = cs0_end; /* CS addressing */

	/* Step 3: Configure DDR type */
	mmdc0->mdmisc = (sysinfo->cs1_mirror << 19) | (sysinfo->walat << 16) |
			(sysinfo->bi_on << 12) | (sysinfo->mif3_mode << 9) |
			(sysinfo->ralat << 6);

	/* Step 4: Configure delay while leaving reset */
	mmdc0->mdor = (txpr << 16) | (sysinfo->sde_to_rst << 8) |
		      (sysinfo->rst_to_cke << 0);

	/* Step 5: Configure DDR physical parameters (density and burst len) */
	coladdr = ddr3_cfg->coladdr;
	if (ddr3_cfg->coladdr == 8)		/* 8-bit COL is 0x3 */
		coladdr += 4;
	else if (ddr3_cfg->coladdr == 12)	/* 12-bit COL is 0x4 */
		coladdr += 1;
	mmdc0->mdctl =  (ddr3_cfg->rowaddr - 11) << 24 |	/* ROW */
			(coladdr - 9) << 20 |			/* COL */
			(1 << 19) |		/* Burst Length = 8 for DDR3 */
			(sysinfo->dsize << 16);		/* DDR data bus size */

	/* Step 6: Perform ZQ calibration */
	val = 0xa1390001; /* one-time HW ZQ calib */
	mmdc0->mpzqhwctrl = val;
	if (sysinfo->dsize > 1)
		MMDC1(mpzqhwctrl, val);

	/* Step 7: Enable MMDC with desired chip select */
	mmdc0->mdctl |= (1 << 31) |			     /* SDE_0 for CS0 */
			((sysinfo->ncs == 2) ? 1 : 0) << 30; /* SDE_1 for CS1 */

	/* Step 8: Write Mode Registers to Init DDR3 devices */
	for (cs = 0; cs < sysinfo->ncs; cs++) {
		/* MR2 */
		val = (sysinfo->rtt_wr & 3) << 9 | (ddr3_cfg->SRT & 1) << 7 |
		      ((tcwl - 3) & 3) << 3;
		debug("MR2 CS%d: 0x%08x\n", cs, (u32)MR(val, 2, 3, cs));
		mmdc0->mdscr = MR(val, 2, 3, cs);
		/* MR3 */
		debug("MR3 CS%d: 0x%08x\n", cs, (u32)MR(0, 3, 3, cs));
		mmdc0->mdscr = MR(0, 3, 3, cs);
		/* MR1 */
		val = ((sysinfo->rtt_nom & 1) ? 1 : 0) << 2 |
		      ((sysinfo->rtt_nom & 2) ? 1 : 0) << 6;
		debug("MR1 CS%d: 0x%08x\n", cs, (u32)MR(val, 1, 3, cs));
		mmdc0->mdscr = MR(val, 1, 3, cs);
		/* MR0 */
		val = ((tcl - 1) << 4) |	/* CAS */
		      (1 << 8)   |		/* DLL Reset */
		      ((twr - 3) << 9) |	/* Write Recovery */
		      (sysinfo->pd_fast_exit << 12); /* Precharge PD PLL on */
		debug("MR0 CS%d: 0x%08x\n", cs, (u32)MR(val, 0, 3, cs));
		mmdc0->mdscr = MR(val, 0, 3, cs);
		/* ZQ calibration */
		val = (1 << 10);
		mmdc0->mdscr = MR(val, 0, 4, cs);
	}

	/* Step 10: Power down control and self-refresh */
	mmdc0->mdpdc = (tcke & 0x7) << 16 |
			5            << 12 |  /* PWDT_1: 256 cycles */
			5            <<  8 |  /* PWDT_0: 256 cycles */
			1            <<  6 |  /* BOTH_CS_PD */
			(tcksrx & 0x7) << 3 |
			(tcksre & 0x7);
	if (!sysinfo->pd_fast_exit)
		mmdc0->mdpdc |= (1 << 7); /* SLOW_PD */
	mmdc0->mapsr = 0x00001006; /* ADOPT power down enabled */

	/* Step 11: Configure ZQ calibration: one-time and periodic 1ms */
	val = 0xa1390003;
	mmdc0->mpzqhwctrl = val;
	if (sysinfo->dsize > 1)
		MMDC1(mpzqhwctrl, val);

	/* Step 12: Configure and activate periodic refresh */
	mmdc0->mdref = (1 << 14) | /* REF_SEL: Periodic refresh cycle: 32kHz */
		       (7 << 11);  /* REFR: Refresh Rate - 8 refreshes */

	/* Step 13: Deassert config request - init complete */
	mmdc0->mdscr = 0x00000000;

	/* wait for auto-ZQ calibration to complete */
	mdelay(1);
}

void mx6_dram_cfg(const struct mx6_ddr_sysinfo *sysinfo,
		  const struct mx6_mmdc_calibration *calib,
		  const void *ddr_cfg)
{
	if (sysinfo->ddr_type == DDR_TYPE_DDR3) {
		mx6_ddr3_cfg(sysinfo, calib, ddr_cfg);
	} else if (sysinfo->ddr_type == DDR_TYPE_LPDDR2) {
		mx6_lpddr2_cfg(sysinfo, calib, ddr_cfg);
	} else {
		puts("Unsupported ddr type\n");
		hang();
	}
}

#endif
