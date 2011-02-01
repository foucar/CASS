
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the RESORT64C_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// RESORT64C_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.




#define LINUX

#ifndef LINUX
	#ifndef WINVER
		#define WINVER 0x0501
	#endif
	#pragma warning(disable : 4005)
#endif

#ifndef NUM_IONS
	#define NUM_IONS (100)  // needed for users who are porting their old projects from the old sort routine to the new one
#endif

#ifndef RESORT_IS_ALREADY_DEFINED
#define RESORT_IS_ALREADY_DEFINED

#define RESORT64C_API

#ifndef LINUX
	#ifdef RESORT64C_EXPORTS
		#define RESORT64C_API __declspec(dllexport)
	#else

// uncomment the following line if linking against the dynamic lib (resort64c.dll):
// comment the line if linking against the static lib of resort64c.lib:

//		#define RESORT64C_API __declspec(dllimport)

	#endif

#else
	#ifndef __int32_IS_DEFINED
		#define __int32_IS_DEFINED
	#define __int32 int
	#define __int16 short
	#define __int64 long long
	#define __int8 char
#endif

#endif


//	List of error_codes of init()-function:
// -1  wrong channel numbers
// -2  uncorrected_time_sum_half_widths not correctly initialized
// -3  corrected_time_sum_half_widths not correctly initialized
// -4  tdc_array_row_length not correctly initialized
// -5  MCP_radius not correctly initialized
// -6  runtime not correctly initialized
// -7  scalefactors not correctly initialized
// -8  error allocating memory for internal arrays
// -9  error during initialization of sum correctors
// -10 error during initialization of pos correctors
// -11 address of count-array not given
// -12 address of tdc-array not given
// -13 TDC binsize not given


class sort_class;
class profile_class;
class gaussfit_class;
class hit_class;
class interpolate_class;
class sum_walk_calibration_class;
class scalefactors_calibration_class;
class nonlinearity_calibration_class;
#ifdef USE_DOUBLE_ARRAY_CLASS
class double_array_class;
#endif




#ifdef USE_DOUBLE_ARRAY_CLASS
class RESORT64C_API double_array_class {
public:
   double_array_class();   // CONSTRUCTOR
   ~double_array_class();
   double& operator[]( __int32 nSubscript );
   double *pointer;
   __int32 * pointer_to_cnt;
};





class RESORT64C_API double_array_pointer_class {
public:
   double_array_pointer_class();  // CONSTRUCTOR
   ~double_array_pointer_class();
   double_array_class& operator[]( __int32 nSubscript );
   double_array_class pointer[7];
};
#endif










class RESORT64C_API version_number_class
{
public:
	__int32 A;
	__int32 B;
	__int32 C;
	__int32 D;
};










class RESORT64C_API signal_corrector_class
{
public:
	signal_corrector_class(); // FUNCTION
	virtual ~signal_corrector_class();	// FUNCTION
	virtual signal_corrector_class * clone();	// FUNCTION
	
	virtual double correct_sum(double t1, double t2,__int32 number_of_layer);  // FUNCTION
	virtual double correct_pos(double t1, double t2,__int32 number_of_layer);  // FUNCTION
	virtual double get_other_anode_signal_corrected(double tmcp_ns, double t_ns,__int32 left_or_right_side_of_output_signal, __int32 layer);  // FUNCTION
	virtual double get_this_anode_signal_corrected(double tmcp_ns, double t_ns,__int32 left_or_right_side_of_input_signal, __int32 layer);  // FUNCTION
	virtual __int32 configure(double max_runtime,double TDC_resolution_ns,bool use_HEX,bool use_pos_correction,bool use_sum_correction); // FUNCTION
	virtual double find_complement_raw_signal(double tmcp_ns, double t_ns, __int32 side_of_input_signal, interpolate_class * corrector); // FUNCTION

	interpolate_class * sum_corrector_U;
	interpolate_class * sum_corrector_V;
	interpolate_class * sum_corrector_W;

	interpolate_class * pos_corrector_U;
	interpolate_class * pos_corrector_V;
	interpolate_class * pos_corrector_W;

	interpolate_class * single_anode_signal_corrector[6];
	interpolate_class * single_partner_anode_signal_corrector[6];

	double TDC_resolution_ns;
	double max_runtime;
	bool   correctors_are_initialized;
	bool   use_HEX;

	bool   use_pos_correction;
	bool   use_sum_correction;

private:

	virtual void init_single_correctors(); // FUNCTION
};










class RESORT64C_API nonlinearity_calibration_class
{
public:
	nonlinearity_calibration_class(__int32 number_of_stages, __int32 number_of_columns, double fu, double fv, double fw, double tdc_resolution, double max_runtime);  // CONSTRUCTOR
	virtual ~nonlinearity_calibration_class();		// FUNCTION

	virtual void finalize();						// FUNCTION
	virtual void fill_pos_histograms(double u1, double u2, double v1, double v2, double w1, double w2, double tmcp); // FUNCTION
	virtual void generate_pos_correction_profiles();	// FUNCTION
	virtual double get_ratio_of_full_bins();		// FUNCTION
	virtual bool map_is_full_enough();				// FUNCTION
	virtual double get_corrected_position_ns(double pos_ns,__int32 layer); // FUNCTION
	virtual double get_corrected_anode_signal_ns(double tmcp_ns,  double signal_ns, __int32 side_of_input_signal, __int32 layer); // FUNCTION
	virtual bool   get_correction_point(double &pos_ns, double &correction_ns, __int32 index, __int32 layer); // FUNCTION
	virtual double get_corrected_partner_anode_signal_ns(double tmcp_ns, double t_ns, __int32 side_of_output_signal, __int32 layer); // FUNCTION


//	virtual nonlinearity_calibration_class * clone();	// FUNCTION

	__int32		detector_map_size;
	double		fu,fv,fw;
	__int32		number_of_columns;
	double		tdc_resolution;
	__int32		number_of_stages;
	__int32		full_bins_counter;
	__int32		bins_with_1_fill_counter;

	profile_class ** pos_profile[3];
	profile_class * single_corrector_profiles[6];
	profile_class * single_complement_corrector_profiles[6];
	sum_walk_calibration_class * sum_walk_calib;
	interpolate_class * correction[3];

	bool		initialization_successful;

private:
	virtual bool reset();							// FUNCTION
	__int32		bins_in_usable_area;
	__int32		full_bin_limit;
	__int16	**	detector_map_counter;
	double      runtime;
	float *		stack[6];
	__int32		stack_counter;
	__int32		stack_size;
};












class RESORT64C_API peak_tracker_class
{
public:
	static peak_tracker_class * new_peak_tracker_class(__int32 stack_size, double center_left_bin, double center_right_bin, __int32 number_of_bins);  // static CONSTRUCTOR
	virtual ~peak_tracker_class();		// FUNCTION
	virtual void reset();				// FUNCTION
	virtual double track(double tracked_value);	// FUNCTION
	virtual bool is_new_result_available(); // FUNCTION

	__int32 stack_size;      
	double  center_left_bin; 
	double  center_right_bin; 
	__int32 number_of_bins;
	double  binsize;

	__int32 * internal_histogram;
	__int32 internal_hit_counter;

	__int32 underflows;
	__int32 overflows;
	double  last_return_value;
	double  maximum_return_up_to_now;
	bool    new_result_available;
	double  maximum_total_correction_up_to_now;
	double  sum_of_all_corrections_up_to_now;
	double	offset_to_user_in_first_run;
	bool    this_is_the_first_run;
	__int32		number_of_resets;

private:
	peak_tracker_class(__int32 stack_size, double center_left_bin, double center_right_bin, __int32 number_of_bins);  // CONSTRUCTOR
};











class RESORT64C_API sum_walk_calibration_class
{

public:
	static sum_walk_calibration_class * new_sum_walk_calibration_class(sort_class* mother_sort_instance, __int32 number_of_columns);  // CONSTRUCTOR
	sum_walk_calibration_class(__int32 number_of_columns, bool use_HEX, double max_runtime_ns,double tdc_resolution); // CONSTRUCTOR
	virtual sum_walk_calibration_class * clone();	// FUNCTION

	virtual ~sum_walk_calibration_class();			// FUNCTION

	virtual void finalize();						// FUNCTION
	virtual bool init_with_sorter(sort_class * mother_sort_instance);				// FUNCTION
	virtual void fill_sum_histograms();				// FUNCTION
	virtual void fill_sum_histograms(double u1, double u2, double v1, double v2, double w1, double w2, double mcp_ns); // FUNCTION
	virtual void fill_sum_histograms(double u1, double u2, double v1, double v2, double mcp_ns); // FUNCTION
	virtual void generate_sum_walk_profiles();		// FUNCTION
	virtual double get_corrected_sum_ns(double sum_ns, double pos_ns,__int32 layer);	// FUNCTION
	virtual bool   get_correction_point(double &pos_ns, double &correction_ns, __int32 index, __int32 layer);	// FUNCTION

	sort_class * sorter;

	__int32 number_of_columns;

	profile_class * sumu_profile;
	profile_class * sumv_profile;
	profile_class * sumw_profile;

	bool initialization_successful;

	double	tdc_resolution;

private:
	sum_walk_calibration_class(sort_class* mother_sort_instance, __int32 number_of_columns);  // FUNCTION
};










class RESORT64C_API scalefactors_calibration_class
{

public:
	
	scalefactors_calibration_class(bool BuildStack,double runtime, double runtime_inner_limit, double fu,double fv,double fw);  // CONSTRUCTOR
	virtual ~scalefactors_calibration_class();  // FUNCTION

	virtual void   feed_calibration_data(double u_ns, double v_ns, double w_ns, double w_ns_minus_woffset);  // FUNCTION
	virtual void   do_auto_calibration(double w_offset);  // FUNCTION
	virtual double get_ratio_of_full_bins();  // FUNCTION
	virtual bool   map_is_full_enough(); // FUNCTION
	
	virtual scalefactors_calibration_class * clone();  // FUNCTION

	double	fu,fv,fw;

	double	best_fv, best_fw, best_w_offset;

	double	detector_map_fill;
	__int32	binx,biny;
	__int32 detector_map_size;
	__int32 full_bins_counter;
	__int32 bins_with_1_fill_counter;

	bool disable_optimization_of_fv;
	bool disable_optimization_of_fw;
	bool disable_optimization_of_offset;
	bool new_was_succesful;
	
private:
	virtual double integrate_deviations(double fv, double fw, double w_offset);  // FUNCTION

	__int32 full_bin_limit;
	bool	BuildStack;
	double	runtime;
	double	runtime_inner_limit;
	float * u_ns_stack;
	float * v_ns_stack;
	float * w_ns_stack;
	float * X_calibration;
	float * Y_calibration;
	__int16	**detector_map_counter;
	float	**detector_map;
	__int32	TDC_stack_counter;
	__int32 tdc_stack_size;
	__int32 bins_in_inner_area;	
};










class RESORT64C_API gaussfit_class
{

public:
	gaussfit_class();// CONSTRUCTOR
	gaussfit_class(double * double_y_array,__int32 number_of_points);// CONSTRUCTOR
	gaussfit_class(__int32 * int_y_array,__int32 number_of_points);// CONSTRUCTOR
	gaussfit_class(double * double_y_array,double fwhm,double x_pos,double height,__int32 number_of_points);// CONSTRUCTOR
	gaussfit_class(__int32 * int_y_array,double fwhm,double x_pos,double height,__int32 number_of_points);// CONSTRUCTOR
	virtual ~gaussfit_class();



	virtual bool do_gauss_fit();  // FUNCTION
	virtual bool find_good_start_values();  // FUNCTION
	virtual void reset();  // FUNCTION
	virtual void smooth();  // FUNCTION
	virtual		 gaussfit_class * clone();  // FUNCTION

	virtual double get_random_gauss_value();  // FUNCTION
	virtual double get_random_value();  // FUNCTION

	__int32		number_of_points;
	double*		double_y_array;
	double		fwhm,height,x_pos;
	double		fwhm_stepsize,height_stepsize,x_pos_stepsize;
	__int32		max_number_of_iterations;

private:
	bool private_array_used;
	virtual void copy_to_private_double_array(__int32 *);  // FUNCTION
	virtual void copy_to_private_double_array(double *);  // FUNCTION
	virtual double calculate_error(double, double, double);  // FUNCTION
	virtual double gauss_value(double , double , double , double);  // FUNCTION
};









class RESORT64C_API profile_class
{

public:
	profile_class();			// CONSTRUCTOR
	virtual ~profile_class();

	virtual void generate_profile_with_gauss();  // FUNCTION
	virtual void generate_profile_with_box();  // FUNCTION
	virtual void generate_profile_with_averaging(); // FUNCTION
	virtual bool box_fit(double * fit_array, __int32 number_of_bins, double &pos, double &width); // FUNCTION
	virtual void reset();  // FUNCTION
	virtual void fill_matrix(double x_pos,double y_pos,double value);  // FUNCTION
	virtual void fill_matrix(__int32 x_bin,__int32 y_bin,double value);  // FUNCTION
	virtual void fill_matrix(double x_pos,double y_pos);  // FUNCTION
	virtual void fill_matrix(__int32 x_bin,__int32 y_bin);  // FUNCTION
	virtual void clear_matrix();  // FUNCTION
	virtual double	 get_x_bin_as_double(double x_pos);  // FUNCTION
	virtual double	 get_y_bin_as_double(double y_pos);  // FUNCTION
	virtual __int32	 get_x_bin(double x_pos);  // FUNCTION
	virtual __int32	 get_y_bin(double y_pos);  // FUNCTION

	virtual double get_bin_center_x(double x_bin);  // FUNCTION
	virtual double get_bin_center_y(double y_bin);  // FUNCTION

	virtual double get_bin_center_x(__int32 x_bin);  // FUNCTION
	virtual double get_bin_center_y(__int32 y_bin);  // FUNCTION

	virtual double get_y(__int32 bin);  // FUNCTION
	virtual double get_fwhm(__int32 bin);  // FUNCTION
	virtual double get_y(double x_pos);		// FUNCTION
	virtual double get_fwhm(double x_pos);	// FUNCTION
	virtual unsigned __int32 get_bin_content(__int32 bin);	// FUNCTION
	virtual unsigned __int32 get_bin_content(double  x_pos);	// FUNCTION
	virtual void	heal_gaps_where_fit_did_not_work(); // FUNCTION
	virtual			profile_class * clone();  // FUNCTION
	

	virtual void transfer_to_interpolate_class_instance(interpolate_class * interpolate_class_instance); // FUNCTION

	__int32		number_of_columns;
	__int32		number_of_rows;
	
	double	center_of_upper_bin;
	double	center_of_lower_bin;
	double	center_of_left_bin;
	double	center_of_right_bin;

	double *	double_profile_y;
	double *	double_profile_fwhm;
	bool   *	bool_profile_fit_worked;
	double **	double_source_matrix;
	unsigned __int32 * bin_content;

private:
	//double dNaN;
	bool using_internal_matrix;
	bool profile_exists;

};












class RESORT64C_API interpolate_class
{

	friend class sort_class;


public:
	
	interpolate_class();									// CONSTRUCTOR
	virtual ~interpolate_class();

	virtual double	get_y(double x_pos);					// FUNCTION
	void			set_point(double x_pos, double y_pos);  // FUNCTION
	void			reset();								// FUNCTION
	virtual			interpolate_class * clone();			// FUNCTION

	double			min_x,max_x;
	
	__int32			number_of_bins_per_nanosecond;
	
	__int32			number_of_points;
	double			binsize_ns;
	bool			is_sorted;

	bool			use_fast_interpolation;
	bool			init_successful;

private:
	void			init_after_setting_points();			// FUNCTION  // must be called after the points have been fed in
	double			interpolate(double x_pos);  // FUNCTION
	void			sort();						// FUNCTION
	void			find_duplicates();			// FUNCTION
	__int32 		size_of_array;
	__int32 		size_of_input_arrays;
	double*			x_input;
	double*			y_input;
	double* 		y_pos;
};









class RESORT64C_API hit_class
{

	friend class sort_class;



public:
	
	hit_class(sort_class *);	// CONSTRUCTOR
	virtual ~hit_class();

	

	__int32		iCu1,iCu2,iCv1,iCv2,iCw1,iCw2,iCmcp;	

	double	fu,fv,fw;

	double	uncorrected_time_sum_half_width_u,uncorrected_time_sum_half_width_v,uncorrected_time_sum_half_width_w;
	double	corrected_time_sum_half_width_u,corrected_time_sum_half_width_v,corrected_time_sum_half_width_w;

	__int32		number_of_reconstructed_signals_in_deadtime_shadow;

	double	tu1, tu2;
	double	tv1, tv2;
	double	tw1, tw2;
	double	tmcp;
	double	ctu1, ctu2;
	double	ctv1, ctv2;
	double	ctw1, ctw2;

	double	tsummcp;
	bool	tsummcp_exists;

	double	x, y, time;

	bool	use_HEX;

	__int32		set_of_rules;

	sort_class * sort_instance;
	bool		common_start_mode;

#ifdef USE_DOUBLE_ARRAY_CLASS
	double_array_pointer_class tdc;
#else
	double	*	tdc[7];
#endif

	__int32		*cnt;

	__int32		number_of_possible_reflections;
	__int32		number_of_reconstructed_signals_sitting_on_top_of_a_real_signal;
	//__int32 number_of_already_used_signals;

	bool		hit_is_good;
	__int32		number_of_combination;

	__int16	*   used_signals[7];
	double	*   reconstructed_signals[7];
	__int32		reconstructed_cnt[7];

	__int32		number_of_original_anode_signals;
	bool		with_mcp;

	unsigned __int32 DEBUG_event_number;
	unsigned __int32 unique_hit_identifier;

	__int32		method;

	__int32		sum_stack_size;
	double	*   u_sum_stack1, *v_sum_stack1, *w_sum_stack1;
	double	*   u_sum_stack2, *v_sum_stack2, *w_sum_stack2;
	__int32	*	u_sum_valid, *v_sum_valid, *w_sum_valid;
	__int32	*	sum_killers;

	__int32		number_of_destroyed_sums;

	bool		hit_was_treated_by_stolen_signal_function;
	__int32		channel_treated_by_stolen_signal_function;

#ifdef USE_DOUBLE_ARRAY_CLASS
	double_array_class	tdc_Cu1,tdc_Cu2,tdc_Cv1,tdc_Cv2,tdc_Cw1,tdc_Cw2,tdc_Cmcp;
#else
	double	*tdc_Cu1,*tdc_Cu2,*tdc_Cv1,*tdc_Cv2,*tdc_Cw1,*tdc_Cw2,*tdc_Cmcp;
#endif
	__int16	*used_signals_Cu1,*used_signals_Cu2,*used_signals_Cv1,*used_signals_Cv2,*used_signals_Cw1,*used_signals_Cw2,*used_signals_Cmcp;

	virtual __int32 get_number_of_signals_which_are_used_more_than(__int32); // FUNCTION

private:
	bool	corrected_anode_signals_are_computed;
	virtual void generate(__int32 iCu1, __int32 iCu2, __int32 iCv1, __int32 iCv2, __int32 iCw1, __int32 iCw2, __int32 iCmcp, __int32 number_of_combination, __int32 unique_hit_identifier,__int32 set_of_rules);  // FUNCTION
	virtual bool is_in_deadtime_shadow(double , __int32);  // FUNCTION
	virtual bool is_closer_to_a_real_signal_than(double , __int32 , double); // FUNCTION
	virtual bool could_be_a_reflection(__int32 , __int32);  // FUNCTION
	virtual bool are_N_signals_used_more_than(__int32,__int32); // (including own signals of this hit) FUNCTION
	virtual void clone(hit_class *); // FUNCTION
	virtual void apply_filter_rules(); // FUNCTION
	virtual __int32 get_method_number(__int32,__int32,__int32,__int32,__int32,__int32,__int32); // FUNCTION
	virtual __int32 how_many_reconstructed_pairs_at_MCP_edge(); // FUNCTION
	virtual __int32 get_number_of_destroyed_sums();  // FUNCTION
	virtual bool check_if_a_used_anode_signal_belongs_to_a_destroyed_sum();  // FUNCTION
	virtual void compute_time();  // FUNCTION
	virtual bool signal_is_distorted(__int32,__int32,double,double);  // FUNCTION
};




















class RESORT64C_API sort_class
{

public:
    
	sort_class();			// CONSTRUCTOR
    virtual ~sort_class();

	virtual __int32	sort();  // FUNCTION
	virtual __int32 run_without_sorting();  // FUNCTION
	
	virtual void sort_an_hit_array_in_time(hit_class* hit_array[], __int32 number_of_hits, bool small_numbers_first);  // FUNCTION
//	virtual bool expand_an_hit_array(hit_class**[], __int32 * , __int32); // FUNCTION
	
	virtual __int32 init_after_setting_parameters(); // FUNCTION
	virtual void get_error_text(__int32 error_code,__int32 buffer_length,char* destination_array);

	
	virtual void shift_layer_w(__int32 add_or_sub_sign,double w_offset);  // FUNCTION
	virtual void shift_sums(__int32 add_or_sub_sign,double sumu_offset,double sumv_offset);  // FUNCTION
	virtual void shift_sums(__int32 add_or_sub_sign,double sumu_offset,double sumv_offset,double sumw_offset);  // FUNCTION
	virtual void shift_position_origin(__int32 add_or_sub_sign,double pos_x_mm_offset,double pos_y_mm_offset);  // FUNCTION
	virtual sort_class * clone();  // FUNCTION

	static version_number_class get_version_number(); // FUNCTION

	// the following variables must be initialized by the user:

	__int32	*count;
	double	*tdc_pointer;
	double	fu,fv,fw;

	double	runtime_u,runtime_v,runtime_w;
	//or
	double	max_runtime;

	__int32	tdc_array_row_length;

#ifdef USE_DOUBLE_ARRAY_CLASS
	double_array_pointer_class tdc;
#else
	double	*tdc[7];
#endif

	__int32	cnt[7];
	double	uncorrected_time_sum_half_width_u,uncorrected_time_sum_half_width_v,uncorrected_time_sum_half_width_w;
	bool	common_start_mode;  // true = common start, false = common stop
	double	MCP_radius;
	bool	use_HEX;
	bool	use_MCP;
	double  TDC_resolution_ns;
	__int32	Cu1,Cu2,Cv1,Cv2,Cw1,Cw2,Cmcp;



	//optional:
	double	corrected_time_sum_half_width_u,corrected_time_sum_half_width_v,corrected_time_sum_half_width_w;
	double	dead_time_anode;
	double	dead_time_mcp;
	bool	use_sum_correction;
	bool	use_pos_correction;
	bool	run_without_sorting_flag;
	bool	dont_overwrite_original_data;

	bool	use_reflection_filter_on_u1,use_reflection_filter_on_u2;
	bool	use_reflection_filter_on_v1,use_reflection_filter_on_v2;
	bool	use_reflection_filter_on_w1,use_reflection_filter_on_w2;

	double  u1_reflection_time_position, u1_reflection_half_width_at_base;
	double  u2_reflection_time_position, u2_reflection_half_width_at_base;
	double  v1_reflection_time_position, v1_reflection_half_width_at_base;
	double  v2_reflection_time_position, v2_reflection_half_width_at_base;
	double  w1_reflection_time_position, w1_reflection_half_width_at_base;
	double  w2_reflection_time_position, w2_reflection_half_width_at_base;


	//the following variables should not be modified:

	double	pos_check_radius_mm;
	double	triple_MCP_time_tolerance_ns;
	double	lower_dead_time_margin;

	__int32	maximum_uses_of_signals;

	//double pre_dead_time;
	__int32	maximum_of_already_used_signals_in_one_hit;

	bool	initialization_successful;
	
	double  * reconstructed_signals[7];
	__int32	  reconstructed_cnt[7];
	__int16	* used_signals[7];

	__int32		output_hit_array_counter;
	hit_class ** output_hit_array;
	
	unsigned __int32 unique_hit_identifier;

	__int32		DEBUG_flag;
	unsigned __int32 DEBUG_event_number;

	version_number_class version_number;
	
#ifdef USE_DOUBLE_ARRAY_CLASS
	double_array_class	tdc_Cu1,tdc_Cu2,tdc_Cv1,tdc_Cv2,tdc_Cw1,tdc_Cw2,tdc_Cmcp;
#else
	double	*tdc_Cu1,*tdc_Cu2,*tdc_Cv1,*tdc_Cv2,*tdc_Cw1,*tdc_Cw2,*tdc_Cmcp;
#endif

	__int16	*used_signals_Cu1,*used_signals_Cu2,*used_signals_Cv1,*used_signals_Cv2,*used_signals_Cw1,*used_signals_Cw2,*used_signals_Cmcp;
	
	__int32	local_sort_instance_id;
	
	signal_corrector_class * signal_corrector;

private:

	virtual bool create_new_hit_instance(__int32 &,__int32 &, hit_class **&,__int32 &); // FUNCTION
	virtual bool detect_twins(); // FUNCTION
	virtual bool get_combination(__int32 , __int32 , bool);   // FUNCTION
	virtual void search_combinations(__int32,bool,__int32,__int32,__int32);   // FUNCTION
	virtual bool init_arrays();  // FUNCTION
	virtual void delete_arrays();  // FUNCTION
	virtual void sort_into_output_hit_array();  // FUNCTION
	virtual void get_all_from_one_combination(__int32,__int32,__int32);  // FUNTCION
	virtual bool garbage_was_found();  // FUNTCION
	virtual __int32 find_next_hit_by_signal(__int32, __int32, __int32, hit_class * [], __int32);  // FUNTCION
	virtual void remove_hit_from_output_hit_array(__int32);  // FUNTCION
	virtual void kill_all_hits_using_this_signal_except(__int32,__int32,__int32*);  // FUNTCION
	virtual void find_stolen_anode_signals(hit_class*[],__int32); // FUNCTION

	static __int32 global_instance_id_counter;

	bool		ready_to_run;
	__int32		number_of_already_used_signals;
	double		tdc_signal_dummy;
	__int32		number_of_blocked_combinations;
	unsigned __int32 * blocked_combinations[7];
	__int32		temp_hit_array_counter;
	hit_class ** temp_hit_array;
	__int32		number_of_hit_instances_in_temp_hit_array_stack;
	__int32		number_of_hit_instances_in_output_hit_array_stack;

	__int32		combination[7];

	__int32		size_of_temp_hit_array_stack;
	__int32		size_of_output_hit_array_stack;


};

#endif



#ifdef iwfh938qh4f9q8hf9hq98h498hfq98hq98h

Help section:



How to use the "peak_tracker_class" by Achim Czasch:

make an instance of the peak_tracker_class:

example:
peak_tracker_class * tracker = peak_tracker_class::new_peak_tracker_class(20000,-w,w,int(2.*w/0.025+1.e-6)+1);

this will initialze "tracker".

Do not forget to delete tracker at the end of the prgram to prevent memory leaks:

delete tracker;

"tracker" will have an internal histogram.
The center of the left bin will be at -w.
The center of teh right bin will be at +w.
The histogram is divided into
int(2.*w/0.025+1.e-6)+1 bins.

after 20000 fill commands "tracker" will calculate the center of mass
of the histogram and then clear the histogram. This is called one "cycle".

Before "tracker" can be used it needs some information.
When you determine the position of a peak in a histogram you will
probably get a slightly different results than the simple COM-calucaltion.
Therefore you must tell "tracker" about this offset.
This is done with

tracker->offset_to_user_in_first_run = x

The best way to get "x" is to create a histogram and fill
it only with the first 20000 events (the first cycle).
Then determine the position and feed the offset into "tracker".

To prevent some pitfalls it is best to track a value which
should be at zero.

Now you can start to fill values into "tracker":

tracker->track(value);

Each time after such a fill you can ask tracker if it has
produced a new correction offset (in other words
"if it has finished one cycle"):

if(tracker->is_new_result_available()) offset = offset - tracker->last_return_value;

tracker->reset() clears the internal histogram.

You can keep track of what "tracker" is doing by looking at the
following member variables:

bool   tracker->this_is_the_first_run (true during first cycle. False after first cycle.)
int    tracker->internal_hit_counter (reset after each cycle)
double tracker->maximum_return_up_to_now (maximum return value of all cycles)
double tracker->maximum_total_correction_up_to_now (maximum total correction since start)
int    tracker->number_of_resets
int    tracker->overflows  (reset after each cycle)
int    tracker->underflows (reset after each cycle)
double tracker->sum_of_all_corrections_up_to_now (sum of all corrections. This is different from "maximum_total_correction_up_to_now".)

The value "tracker->maximum_total_correction_up_to_now" helps to check
wether a value has drifted too far.
#endif
