#ifndef __CycleCounter_H
#define __CycleCounter_H

#ifdef __cplusplus
extern "C" {
#endif

#define  ELAPSED_TIME_MAX_SECTIONS  10

void  elapsed_time_clr(uint32_t i);		// Clear measured values
void  elapsed_time_init(void);			// Module initialization
void  elapsed_time_start(uint32_t i);	// Start measurement 
void  elapsed_time_stop(uint32_t i);	// Stop  measurement 

#ifdef __cplusplus
}
#endif

#endif /* __v_H */