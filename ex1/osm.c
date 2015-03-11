/**
 * @file osm.c
 * @author  griffonn ednussi
 * @version 1.0
 * @date 10 Mar 2015
 * 
 * @brief Library for measuring different basic operations time
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * This library provides functionality for measures time of simple arithmetic 
 * operation, empty system call, and empty function call, and calculates ratios 
 * between them.
 */
#include "osm.h"
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>

#define DEFAULT_ITERATION_NUMBER 50000
#define DEFAULT_ITER_OPERATIONS_NUMBER 10
#define MEGA pow(10, 3)
#define GIGA pow(10, 6)
#define DEFAULT_BAD_RESULT -1
#define WORKING_PROPERLY 0

int main()
{	
	return WORKING_PROPERLY;
}

/**
 * Initialization function that the user must call
 * before running any other library function.
 * Returns 0 as does nothing, to be consistent with the API.
 */
int osm_init()
{
	return WORKING_PROPERLY;
}

/**
 *  @brief Calculates difference between two time points given in timeval 
 *		format, then measures average time of iteration for given iteration 
 *		number.
 *  @param startTime a timeval struct with starting time
 *  @param stopTime a timeval struct with stop time
 *  @param osm_iterations number of iterations
 *  @return average time of one iteration in nanoseconds
 */
double calculateTimeDifference(struct timeval startTime, 
							   struct timeval stopTime)
{
	double sec_diff = stopTime.tv_sec - startTime.tv_sec;
	double msec_diff = stopTime.tv_usec - startTime.tv_usec;

	// We need in nanoseconds
	return MEGA * (sec_diff * GIGA + msec_diff);
}

/**
 *  @brief Calculates average time of double gettimeofday call.
 *  @param iterations number of iterations
 *  @return average time of one iteration in nanoseconds
 */
double avgGettimeofdayCallTime(unsigned int iterations)
{
	struct timeval startTime;
	struct timeval stopTime;

	unsigned int i;
	for (i = 0; i < iterations; ++i)
	{
		if (gettimeofday(&startTime, NULL))
		{
			return DEFAULT_BAD_RESULT;
		}
		if (gettimeofday(&stopTime, NULL))
		{
			return DEFAULT_BAD_RESULT;
		}
	}

	return calculateTimeDifference(startTime, stopTime) / iterations;
}

/**
 *  @brief Calculates average time of incrementing a loop counter variable.
 *  @param iterations number of iterations
 *  @return average time of one iteration in nanoseconds
 */
double avgForLoopRuntime(unsigned int iterations)
{
	struct timeval startTime;
	struct timeval stopTime;

	unsigned int i;
	
	if (gettimeofday(&startTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}

	for (i = 0; i < iterations; i += DEFAULT_ITER_OPERATIONS_NUMBER);

	if (gettimeofday(&stopTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}

	return  calculateTimeDifference(startTime, stopTime) / iterations;
}

/**
 *  @brief Empty function
 */
void emptyFunc()
{

}

/** 
 *  @brief Checks if the given iteration number is valid.
 *  @param osm_iterations number of iterations
 *  @return 0 if the osm_iteration is valid
 */
int isValidIterationsNumber(unsigned int osm_iterations)
{
	return (osm_iterations != 0 && osm_iterations < INT_MAX);
}

/** 
 *  @brief Rounds up the osm_interations number to fit unrolling defaults.
 *  @param osm_iterations number of iterations
 *  @return 0 if the osm_iteration is valid
 */
int normalizeIterationsNumber(unsigned int osm_iterations)
{
	int remaider = osm_iterations % DEFAULT_ITER_OPERATIONS_NUMBER;

	if (remaider != 0)
	{
		osm_iterations += DEFAULT_ITER_OPERATIONS_NUMBER - remaider;
	}

	return osm_iterations;
}

/**
 *  @brief Time measurement function for an empty function call.
 *  	returns time in nano-seconds upon success, 
 *  	and -1 upon failure.
 *  	Zero iterations number is invalid.
 *  @param osm_iterations number of iterations
 *  @return average time for empty function call
 */
double osm_function_time(unsigned int osm_iterations)
{
	// Validate
	if (!isValidIterationsNumber(osm_iterations))
	{
		osm_iterations = DEFAULT_ITERATION_NUMBER;
	}

	struct timeval startTime;
	struct timeval stopTime;
	unsigned int i;

	// Start time measuring
	if (gettimeofday(&startTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}
	
	for (i = 0; i < osm_iterations; i += DEFAULT_ITER_OPERATIONS_NUMBER)
	{
		emptyFunc();
		emptyFunc();
		emptyFunc();
		emptyFunc();
		emptyFunc();

		emptyFunc();
		emptyFunc();
		emptyFunc();
		emptyFunc();
		emptyFunc();
	}

	if (gettimeofday(&stopTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}
	
	return calculateTimeDifference(startTime, stopTime) / osm_iterations;
}

/**
 *  @brief Time measurement function for an empty trap into the OS.
 *  	returns time in nano-seconds upon success, 
 *  	and -1 upon failure.
 *  	Zero iterations number is invalid.
 *  @param osm_iterations number of iterations
 *  @return average time for empty trap call
   */
double osm_syscall_time(unsigned int osm_iterations)
{
	// Validate
	if (!isValidIterationsNumber(osm_iterations))
	{
		osm_iterations = DEFAULT_ITERATION_NUMBER;
	}

	struct timeval startTime;
	struct timeval stopTime;
	unsigned int i;

	// Start time measuring
	if (gettimeofday(&startTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}

	for (i = 0; i < osm_iterations; i += DEFAULT_ITER_OPERATIONS_NUMBER)
	{
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;

		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
		OSM_NULLSYSCALL;
	}

	if (gettimeofday(&stopTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}

	return calculateTimeDifference(startTime, stopTime) / osm_iterations;
}


/**
 *  @brief Time measurement function for a simple arithmetic operation.
 *  	returns time in nano-seconds upon success,
 *  	and -1 upon failure.
 *  	Zero iterations number is invalid.
 *  @param osm_iterations number of iterations
 *  @return average time for simple operation
 */
double osm_operation_time(unsigned int osm_iterations)
{
	// Validate
	if (!isValidIterationsNumber(osm_iterations))
	{
		osm_iterations = DEFAULT_ITERATION_NUMBER;
	}

	struct timeval startTime;
	struct timeval stopTime;
	unsigned int i;

	// Start time measuring
	if (gettimeofday(&startTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}

	for (i = 0; i < osm_iterations; i += DEFAULT_ITER_OPERATIONS_NUMBER)
	{
		__asm__("movl $1, %eax;" "movl $2, %ebx;" "addl %ebx, %eax;");
		__asm__("movl $1, %eax;" "movl $2, %ebx;" "addl %ebx, %eax;");
		__asm__("movl $1, %eax;" "movl $2, %ebx;" "addl %ebx, %eax;");
		__asm__("movl $1, %eax;" "movl $2, %ebx;" "addl %ebx, %eax;");
		__asm__("movl $1, %eax;" "movl $2, %ebx;" "addl %ebx, %eax;");

		__asm__("movl $1, %eax;" "movl $2, %ebx;" "addl %ebx, %eax;");
		__asm__("movl $1, %eax;" "movl $2, %ebx;" "addl %ebx, %eax;");
		__asm__("movl $1, %eax;" "movl $2, %ebx;" "addl %ebx, %eax;");
		__asm__("movl $1, %eax;" "movl $2, %ebx;" "addl %ebx, %eax;");
		__asm__("movl $1, %eax;" "movl $2, %ebx;" "addl %ebx, %eax;");
	}

	if (gettimeofday(&stopTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}

	return calculateTimeDifference(startTime, stopTime) / osm_iterations;
}

/**
 *  @brief Measure times of three different operations:
 *  	- simple operation (addition),
 * 		- empty trap,
 *		- empty function call,
 *		given number of iterations, and save the times it take.
 *	@param osm_iterations number of iterations to perform (positive integer)
 *	@return structure with results
 */
timeMeasurmentStructure measureTimes (unsigned int osm_iterations)
{
	timeMeasurmentStructure results;

	// Get machine data
	int successGetHostName = gethostname(results.machineName, HOST_NAME_MAX);
	if (successGetHostName != 0)
	{
		// Set the hostname to be empty:
		results.machineName[0] = '\0';
	}

	// Validate input
	if (!isValidIterationsNumber(osm_iterations))
	{
		osm_iterations = DEFAULT_ITERATION_NUMBER;
	}

	// Round up the iterations if needed
	osm_iterations = normalizeIterationsNumber(osm_iterations);

	// Calculate utilities time
	double avgGettingTime = avgGettimeofdayCallTime(GIGA);
	double avgForLoopTime = avgForLoopRuntime(GIGA); 

	// Save iteration number to the output struct
	results.numberOfIterations = osm_iterations;

	// Run measurements and substract avg. time of (1) adding to loop counter 
	// and (2) avg. time of gettimeofday() call
	results.instructionTimeNanoSecond = osm_operation_time(osm_iterations) 
										- avgGettingTime - avgForLoopTime;
	
	results.functionTimeNanoSecond = osm_function_time(osm_iterations) 
									 - avgGettingTime - avgForLoopTime;

	results.trapTimeNanoSecond = osm_syscall_time(osm_iterations) 
								 - avgGettingTime - avgForLoopTime;

	// Calculate ratios, set defaults in case of an error
	if (results.functionTimeNanoSecond != DEFAULT_BAD_RESULT && 
		results.instructionTimeNanoSecond != DEFAULT_BAD_RESULT)
	{
		results.functionInstructionRatio = results.functionTimeNanoSecond / 
										   results.instructionTimeNanoSecond;
	}
	else
	{
		results.functionInstructionRatio = DEFAULT_BAD_RESULT;
	}

	if (results.trapTimeNanoSecond != DEFAULT_BAD_RESULT && 
		results.instructionTimeNanoSecond != DEFAULT_BAD_RESULT)
	{
		results.trapInstructionRatio = results.trapTimeNanoSecond / 
									   results.instructionTimeNanoSecond;
	}
	else
	{
		results.trapInstructionRatio = DEFAULT_BAD_RESULT;
	}

	return results;
}
