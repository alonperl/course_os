#include "osm.h"
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>

#define DEFAULT_ITERATION_NUMBER 50000
#define DEFAULT_ITER_OPERATIONS_NUMBER 5
#define MEGA pow(10, 3)
#define GIGA pow(10, 6)
#define TERA pow(10, 9)

#define DEFAULT_BAD_RESULT -1

#define WORKING_PROPERLY 0



int main()
{	
	timeMeasurmentStructure tms = measureTimes(0);
	printf("Hostname:\t\t\t%s\n", tms.machineName);
	printf("Number of iterations:\t\t%d\n", tms.numberOfIterations);

	printf("Function:\t\t\t%f\n", tms.functionTimeNanoSecond);
	printf("Trap:\t\t\t\t%f\n", tms.trapTimeNanoSecond);
	printf("Instruction\t\t\t%f\n", tms.instructionTimeNanoSecond);
	
	printf("Function/Instruction Ratio:\t%f\n", tms.functionInstructionRatio);
	printf("Trap/Instruction Ratio:\t\t%f\n", tms.trapInstructionRatio);

	return WORKING_PROPERLY;
}

/* Initialization function that the user must call
 * before running any other library function.
 * Returns 0 uppon success and -1 on failure.
 */
int osm_init()
{
	return WORKING_PROPERLY;
}

/**
 *  @brief Calculates difference between two time points given in timespec format,
 *  	then measures average time of iteration for given iteration number.
 *  @param startTime a timespec struct with starting time
 *  @param stopTime a timespec struct with stop time
 *  @param osm_iterations number of iterations
 *  @return average time of one iteration in nanoseconds
 */
// double calculateTimeDifference(struct timespec startTime, struct timespec stopTime)
double calculateTimeDifference(struct timeval startTime, struct timeval stopTime)
{
	double sec_diff = stopTime.tv_sec - startTime.tv_sec;
	double msec_diff = stopTime.tv_usec - startTime.tv_usec;

	double ms = sec_diff * GIGA + msec_diff;

	// We need only nanoseconds
	return MEGA * ms;
}

double averageGettingTimeTime(unsigned int iterations)
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

double averageForLoopRuntime(unsigned int iterations)
{
	struct timeval startTime;
	struct timeval stopTime;

	unsigned int i;
	
	if (gettimeofday(&startTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}

	for (i = 0; i < iterations; i+=10)
	{

	}

	if (gettimeofday(&stopTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}

	return  calculateTimeDifference(startTime, stopTime) / iterations;
}

/**
 *  @brief Empty function
 */
void empty()
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

/*  @brief Time measurement function for an empty function call.
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

	// struct timespec startTime;
	// struct timespec stopTime;
	struct timeval startTime;
	struct timeval stopTime;

	// Start time measuring

	unsigned int i;
	
	// if (clock_gettime(CLOCK_REALTIME, &startTime))
	if (gettimeofday(&startTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}
	
	for (i = 0; i < osm_iterations; i+=10)
	{
		empty();
		empty();
		empty();
		empty();
		empty();
		empty();
		empty();
		empty();
		empty();
		empty();
	}

	// if (clock_gettime(CLOCK_REALTIME, &stopTime))
	if (gettimeofday(&stopTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}
	
	// Return difference
	return calculateTimeDifference(startTime, stopTime) / osm_iterations;
}

/*  @brief Time measurement function for an empty trap into the operating system.
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

	// struct timespec startTime;
	// struct timespec stopTime;
	struct timeval startTime;
	struct timeval stopTime;

	// Start time measuring

	unsigned int i;

	// if (clock_gettime(CLOCK_REALTIME, &startTime))
	if (gettimeofday(&startTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}

	for (i = 0; i < osm_iterations; i+=10)
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

	// if (clock_gettime(CLOCK_REALTIME, &stopTime))
	if (gettimeofday(&stopTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}

	// Return difference
	return calculateTimeDifference(startTime, stopTime) / osm_iterations;
}


/*  @brief Time measurement function for a simple arithmetic operation.
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

	// Set operands
	int firstOperand = 1;
	int secondOperand = 2;
	int resultOperand;

	// struct timespec startTime;
	// struct timespec stopTime;
	struct timeval startTime;
	struct timeval stopTime;

	// Start time measuring

	unsigned int i;

	// if (clock_gettime(CLOCK_REALTIME, &startTime))
	if (gettimeofday(&startTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}

	for (i = 0; i < osm_iterations; i+=10)
	{
		1+2;
		1+2;
		1+2;
		1+2;
		1+2;
                1+2;
                1+2;
                1+2;
                1+2;
                1+2;
	}

	// if (clock_gettime(CLOCK_REALTIME, &stopTime))
	if (gettimeofday(&stopTime, NULL))
	{
		return DEFAULT_BAD_RESULT;
	}

	// Dirty hack, see forum.
	resultOperand++;

	// Return difference
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
		// TODO wtf how to set it to null
		results.machineName[0] = '\0';
	}

	if (!isValidIterationsNumber(osm_iterations))
	{
		osm_iterations = DEFAULT_ITERATION_NUMBER;
	}

        double averageGettingTime = averageGettingTimeTime(osm_iterations);
        double averageForLoopTime = averageForLoopRuntime(GIGA); 

	// Set iteration number
	results.numberOfIterations = osm_iterations;

	// Run measurements
	results.instructionTimeNanoSecond = osm_operation_time(osm_iterations);// - averageGettingTime - averageForLoopTime;
	results.functionTimeNanoSecond = osm_function_time(osm_iterations);// - averageGettingTime - averageForLoopTime;
	results.trapTimeNanoSecond = osm_syscall_time(osm_iterations);// - averageGettingTime - averageForLoopTime;

	// Calculate ratios
	if (results.functionTimeNanoSecond != DEFAULT_BAD_RESULT && results.instructionTimeNanoSecond != DEFAULT_BAD_RESULT)
	{
		results.functionInstructionRatio = results.functionTimeNanoSecond / results.instructionTimeNanoSecond;
	}
	else
	{
		results.functionInstructionRatio = DEFAULT_BAD_RESULT;	
	}

	if (results.trapTimeNanoSecond != DEFAULT_BAD_RESULT && results.instructionTimeNanoSecond != DEFAULT_BAD_RESULT)
	{
		results.trapInstructionRatio = results.trapTimeNanoSecond / results.instructionTimeNanoSecond;
	}
	else
	{
		results.trapInstructionRatio = DEFAULT_BAD_RESULT;	
	}

	return results;

}
