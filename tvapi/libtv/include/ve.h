#ifndef __VE_H
#define __VE_H

// ***************************************************************************
// *** enum definitions *********************************************
// ***************************************************************************

typedef enum ve_demo_pos_e {
	VE_DEMO_POS_TOP = 0,
	VE_DEMO_POS_BOTTOM,
	VE_DEMO_POS_LEFT,
	VE_DEMO_POS_RIGHT,
} ve_demo_pos_t;

typedef enum ve_dnlp_rt_e {
	VE_DNLP_RT_0S = 0,
	VE_DNLP_RT_1S = 6,
	VE_DNLP_RT_2S,
	VE_DNLP_RT_4S,
	VE_DNLP_RT_8S,
	VE_DNLP_RT_16S,
	VE_DNLP_RT_32S,
	VE_DNLP_RT_64S,
	VE_DNLP_RT_FREEZE,
} ve_dnlp_rt_t;

// ***************************************************************************
// *** struct definitions *********************************************
// ***************************************************************************

typedef enum ve_dnlp_rl_e {
	VE_DNLP_RL_01 = 1, // max_contrast = 1.0625x
	VE_DNLP_RL_02,     // max_contrast = 1.1250x
	VE_DNLP_RL_03,     // max_contrast = 1.1875x
	VE_DNLP_RL_04,     // max_contrast = 1.2500x
	VE_DNLP_RL_05,     // max_contrast = 1.3125x
	VE_DNLP_RL_06,     // max_contrast = 1.3750x
	VE_DNLP_RL_07,     // max_contrast = 1.4375x
	VE_DNLP_RL_08,     // max_contrast = 1.5000x
	VE_DNLP_RL_09,     // max_contrast = 1.5625x
	VE_DNLP_RL_10,     // max_contrast = 1.6250x
	VE_DNLP_RL_11,     // max_contrast = 1.6875x
	VE_DNLP_RL_12,     // max_contrast = 1.7500x
	VE_DNLP_RL_13,     // max_contrast = 1.8125x
	VE_DNLP_RL_14,     // max_contrast = 1.8750x
	VE_DNLP_RL_15,     // max_contrast = 1.9375x
	VE_DNLP_RL_16,     // max_contrast = 2.0000x
} ve_dnlp_rl_t;

typedef enum ve_dnlp_ext_e {
	VE_DNLP_EXT_00 = 0, // weak
	VE_DNLP_EXT_01,
	VE_DNLP_EXT_02,
	VE_DNLP_EXT_03,
	VE_DNLP_EXT_04,
	VE_DNLP_EXT_05,
	VE_DNLP_EXT_06,
	VE_DNLP_EXT_07,
	VE_DNLP_EXT_08,
	VE_DNLP_EXT_09,
	VE_DNLP_EXT_10,
	VE_DNLP_EXT_11,
	VE_DNLP_EXT_12,
	VE_DNLP_EXT_13,
	VE_DNLP_EXT_14,
	VE_DNLP_EXT_15,
	VE_DNLP_EXT_16,     // strong
} ve_dnlp_ext_t;

typedef struct ve_bext_s {
	unsigned char en;
	unsigned char start;
	unsigned char slope1;
	unsigned char midpt;
	unsigned char slope2;
} ve_bext_t;

typedef struct ve_dnlp_s {
	unsigned int      en;
	enum ve_dnlp_rt_e rt;
	enum ve_dnlp_rl_e rl;
	enum ve_dnlp_ext_e black;
	enum ve_dnlp_ext_e white;
} ve_dnlp_t;
typedef struct ve_hist_s {
	unsigned long sum;
	int width;
	int height;
	int ave;
} ve_hist_t;
typedef struct ve_dnlp_table_s {
	unsigned int en;
	unsigned int method;
	unsigned int cliprate;
	unsigned int lowrange;
	unsigned int hghrange;
	unsigned int lowalpha;
	unsigned int midalpha;
	unsigned int hghalpha;
} ve_dnlp_table_t;
typedef struct ve_hsvs_s {
	unsigned char en;
	unsigned char peak_gain_h1;
	unsigned char peak_gain_h2;
	unsigned char peak_gain_h3;
	unsigned char peak_gain_h4;
	unsigned char peak_gain_h5;
	unsigned char peak_gain_v1;
	unsigned char peak_gain_v2;
	unsigned char peak_gain_v3;
	unsigned char peak_gain_v4;
	unsigned char peak_gain_v5;
	unsigned char peak_gain_v6;
	unsigned char hpeak_slope1;
	unsigned char hpeak_slope2;
	unsigned char hpeak_thr1;
	unsigned char hpeak_thr2;
	unsigned char hpeak_nlp_cor_thr;
	unsigned char hpeak_nlp_gain_pos;
	unsigned char hpeak_nlp_gain_neg;
	unsigned char vpeak_slope1;
	unsigned char vpeak_slope2;
	unsigned char vpeak_thr1;
	unsigned char vpeak_thr2;
	unsigned char vpeak_nlp_cor_thr;
	unsigned char vpeak_nlp_gain_pos;
	unsigned char vpeak_nlp_gain_neg;
	unsigned char speak_slope1;
	unsigned char speak_slope2;
	unsigned char speak_thr1;
	unsigned char speak_thr2;
	unsigned char speak_nlp_cor_thr;
	unsigned char speak_nlp_gain_pos;
	unsigned char speak_nlp_gain_neg;
	unsigned char peak_cor_gain;
	unsigned char peak_cor_thr_l;
	unsigned char peak_cor_thr_h;
	unsigned char vlti_step;
	unsigned char vlti_step2;
	unsigned char vlti_thr;
	unsigned char vlti_gain_pos;
	unsigned char vlti_gain_neg;
	unsigned char vlti_blend_factor;
	unsigned char hlti_step;
	unsigned char hlti_thr;
	unsigned char hlti_gain_pos;
	unsigned char hlti_gain_neg;
	unsigned char hlti_blend_factor;
	unsigned char vlimit_coef_h;
	unsigned char vlimit_coef_l;
	unsigned char hlimit_coef_h;
	unsigned char hlimit_coef_l;
	unsigned char cti_444_422_en;
	unsigned char cti_422_444_en;
	unsigned char cti_blend_factor;
	unsigned char vcti_buf_en;
	unsigned char vcti_buf_mode_c5l;
	unsigned char vcti_filter;
	unsigned char hcti_step;
	unsigned char hcti_step2;
	unsigned char hcti_thr;
	unsigned char hcti_gain;
	unsigned char hcti_mode_median;
} ve_hsvs_t;

typedef struct ve_ccor_s {
	unsigned char en;
	unsigned char slope;
	unsigned char thr;
} ve_ccor_t;

typedef struct ve_benh_s {
	unsigned char en;
	unsigned char cb_inc;
	unsigned char cr_inc;
	unsigned char gain_cr;
	unsigned char gain_cb4cr;
	unsigned char luma_h;
	unsigned char err_crp;
	unsigned char err_crn;
	unsigned char err_cbp;
	unsigned char err_cbn;
} ve_benh_t;

typedef struct ve_cbar_s {
	unsigned char en;
	unsigned char wid;
	unsigned char cr;
	unsigned char cb;
	unsigned char y;
} ve_cbar_t;
typedef struct ve_demo_s {
	unsigned char bext;
	unsigned char dnlp;
	unsigned char hsvs;
	unsigned char ccor;
	unsigned char benh;
	enum  ve_demo_pos_e  pos;
	unsigned long wid;
	struct ve_cbar_s   cbar;
} ve_demo_t;

typedef struct vdo_meas_s {
	//...
} vdo_meas_t;

typedef struct ve_regmap_s {
	unsigned long reg[43];
} ve_regmap_t;

// ***************************************************************************
// *** MACRO definitions **********
// ***************************************************************************

// ***************************************************************************
// *** FUNCTION definitions **********
// ***************************************************************************

#endif  // _VE_H
