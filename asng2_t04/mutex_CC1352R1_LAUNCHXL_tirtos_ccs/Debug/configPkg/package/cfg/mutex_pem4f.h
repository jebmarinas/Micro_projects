/*
 *  Do not modify this file; it is automatically 
 *  generated and any modifications will be overwritten.
 *
 * @(#) xdc-I11
 */

#include <xdc/std.h>

#include <ti/sysbios/heaps/HeapMem.h>
extern const ti_sysbios_heaps_HeapMem_Handle heap0;

#include <ti/sysbios/knl/Semaphore.h>
extern const ti_sysbios_knl_Semaphore_Handle adc_trigger;

#include <ti/sysbios/knl/Semaphore.h>
extern const ti_sysbios_knl_Semaphore_Handle pwm_trigger;

#include <ti/sysbios/knl/Semaphore.h>
extern const ti_sysbios_knl_Semaphore_Handle uart_trigger;

#include <ti/sysbios/knl/Task.h>
extern const ti_sysbios_knl_Task_Handle adc_task;

#include <ti/sysbios/knl/Task.h>
extern const ti_sysbios_knl_Task_Handle uart_task;

#include <ti/sysbios/knl/Task.h>
extern const ti_sysbios_knl_Task_Handle pwm_task;

extern int xdc_runtime_Startup__EXECFXN__C;

extern int xdc_runtime_Startup__RESETFXN__C;

extern int xdc_rov_runtime_Mon__checksum;

extern int xdc_rov_runtime_Mon__write;

