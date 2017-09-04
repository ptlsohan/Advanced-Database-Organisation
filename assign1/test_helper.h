#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern char *testName;


#define TEST_INFO  __FILE__, testName, __LINE__, __TIME__


#define TEST_CHECK(code)						\
  do {									\
    int rc_internal = (code);						\
    if (rc_internal != RC_OK)						\
      {									\
	char *message = errorMessage(rc_internal);			\
	printf("[%s-%s-L%i-%s] FAILED: Operation returned error: %s\n",TEST_INFO, message); \
	free(message);							\
	exit(1);							\
      }									\
  } while(0);


#define ASSERT_EQUALS_STRING(expected,real,message)			\
  do {									\
    if (strcmp((expected),(real)) != 0)					\
      {									\
	printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n",TEST_INFO, expected, real, message); \
	exit(1);							\
      }									\
    printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n",TEST_INFO, expected, real, message); \
  } while(0)


#define ASSERT_EQUALS_INT(expected,real,message)			\
  do {									\
    if ((expected) != (real))					\
      {									\
	printf("[%s-%s-L%i-%s] FAILED: expected <%i> but was <%i>: %s\n",TEST_INFO, expected, real, message); \
	exit(1);							\
      }									\
    printf("[%s-%s-L%i-%s] OK: expected <%i> and was <%i>: %s\n",TEST_INFO, expected, real, message); \
  } while(0)


#define ASSERT_TRUE(real,message)					\
  do {									\
    if (!(real))							\
      {									\
	printf("[%s-%s-L%i-%s] FAILED: expected true: %s\n",TEST_INFO, message); \
	exit(1);							\
      }									\
    printf("[%s-%s-L%i-%s] OK: expected true: %s\n",TEST_INFO, message); \
  } while(0)


#define ASSERT_ERROR(expected,message)		\
  do {									\
    int result = (expected);						\
    if (result == (RC_OK))						\
      {									\
	printf("[%s-%s-L%i-%s] FAILED: expected an error: %s\n",TEST_INFO, message); \
	exit(1);							\
      }									\
    printf("[%s-%s-L%i-%s] OK: expected an error and was RC <%i>: %s\n",TEST_INFO,  result , message); \
  } while(0)


#define TEST_DONE()							\
  do {									\
    printf("[%s-%s-L%i-%s] OK: finished test\n\n",TEST_INFO); \
  } while (0)

#endif
