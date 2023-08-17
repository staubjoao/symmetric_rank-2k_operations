#ifndef POLYBENCH_H
#define POLYBENCH_H
#ifndef POLYBENCH_PADDING_FACTOR
#define POLYBENCH_PADDING_FACTOR 0
#endif

#ifdef POLYBENCH_USE_C99_PROTO
#define POLYBENCH_C99_SELECT(x, y) y
#else
/* default: */
#define POLYBENCH_C99_SELECT(x, y) x
#endif

#ifndef POLYBENCH_STACK_ARRAYS
#define POLYBENCH_ARRAY(x) *x
#define POLYBENCH_FREE_ARRAY(x) free((void *)x);
#define POLYBENCH_DECL_VAR(x) (*x)
#else
#define POLYBENCH_ARRAY(x) x
#define POLYBENCH_FREE_ARRAY(x)
#define POLYBENCH_DECL_VAR(x) x
#endif

#define POLYBENCH_2D(var, dim1, dim2, ddim1, ddim2) var[POLYBENCH_C99_SELECT(dim1, ddim1) + POLYBENCH_PADDING_FACTOR][POLYBENCH_C99_SELECT(dim2, ddim2) + POLYBENCH_PADDING_FACTOR]

#define POLYBENCH_ALLOC_2D_ARRAY(n1, n2, type) \
  (type(*)[n1 + POLYBENCH_PADDING_FACTOR][n2 + POLYBENCH_PADDING_FACTOR]) polybench_alloc_data((n1 + POLYBENCH_PADDING_FACTOR) * (n2 + POLYBENCH_PADDING_FACTOR), sizeof(type))

#ifndef POLYBENCH_STACK_ARRAYS
#define POLYBENCH_2D_ARRAY_DECL(var, type, dim1, dim2, ddim1, ddim2)    \
  type POLYBENCH_2D(POLYBENCH_DECL_VAR(var), dim1, dim2, ddim1, ddim2); \
  var = POLYBENCH_ALLOC_2D_ARRAY(POLYBENCH_C99_SELECT(dim1, ddim1), POLYBENCH_C99_SELECT(dim2, ddim2), type);
#else
#define POLYBENCH_2D_ARRAY_DECL(var, type, dim1, dim2, ddim1, ddim2) \
  type POLYBENCH_2D(POLYB ENCH_DECL_VAR(var), dim1, dim2, ddim1, ddim2);
#endif

/* Dead-code elimination macros. Use argc/argv for the run-time check. */
#ifndef POLYBENCH_DUMP_ARRAYS
#define POLYBENCH_DCE_ONLY_CODE if (argc > 42 && !strcmp(argv[0], ""))
#else
#define POLYBENCH_DCE_ONLY_CODE
#endif

#define polybench_prevent_dce(func) \
  POLYBENCH_DCE_ONLY_CODE           \
  func

#define polybench_start_instruments
#define polybench_stop_instruments
#define polybench_print_instruments

/* PAPI support. */
#ifdef POLYBENCH_PAPI
extern const unsigned int polybench_papi_eventlist[];
#undef polybench_start_instruments
#undef polybench_stop_instruments
#undef polybench_print_instruments
#define polybench_set_papi_thread_report(x) \
  polybench_papi_counters_threadid = x;
#define polybench_start_instruments                           \
  polybench_prepare_instruments();                            \
  polybench_papi_init();                                      \
  int evid;                                                   \
  for (evid = 0; polybench_papi_eventlist[evid] != 0; evid++) \
  {                                                           \
    if (polybench_papi_start_counter(evid))                   \
      continue;

#define polybench_stop_instruments   \
  polybench_papi_stop_counter(evid); \
  }                                  \
  polybench_papi_close();

#define polybench_print_instruments polybench_papi_print();
#endif

#if defined(POLYBENCH_TIME) || defined(POLYBENCH_GFLOPS)
#undef polybench_start_instruments
#undef polybench_stop_instruments
#undef polybench_print_instruments
#define polybench_start_instruments polybench_timer_start();
#define polybench_stop_instruments polybench_timer_stop();
#define polybench_print_instruments polybench_timer_print();
extern double polybench_program_total_flops;
extern void polybench_timer_start();
extern void polybench_timer_stop();
extern void polybench_timer_print();
#endif

/* Function declaration. */
#ifdef POLYBENCH_TIME
extern void polybench_timer_start();
extern void polybench_timer_stop();
extern void polybench_timer_print();
#endif

#ifdef POLYBENCH_PAPI
extern void polybench_prepare_instruments();
extern int polybench_papi_start_counter(int evid);
extern void polybench_papi_stop_counter(int evid);
extern void polybench_papi_init();
extern void polybench_papi_close();
extern void polybench_papi_print();
#endif

/* Function prototypes. */
extern void *polybench_alloc_data(int n, int elt_size);

#endif /* !POLYBENCH_H */
