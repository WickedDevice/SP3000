/*
 * debug.hpp
 *
 *  Created on: 27 okt 2013
 *      Author: pontus
 *
 *  The debug macro's in this file is enabled by a compile time flag called
 *  DEBUG. This flag can be set on the command line or by setting it in the
 *  source file before including this header file.
 *
 *  The debug macro's PRINT and PRINTLN are fully exclusive so when the DEBUG
 *  flag is disabled there will be no code generated. This can cause if
 *  statements to fail on the following constructs
 *
 *  <code>
 *  if (flag < 0)
 *    PRINT ("Error !");
 *  flag++;
 *  </code>
 *
 *  In the case above when the debug flag is disabled the if statement will
 *  think that the source line "flag++;" is the conditional true statement
 *  instead of the "PRINT ("Error !");" line.
 *
 *  <code>
 *  if (flag < 0) {
 *    PRINT ("Error !");
 *  }
 *  flag++;
 *  </code>
 *
 *  In this case the compiler will throw an error since it will not allow
 *  empty code blocks enclosed by "{" and "}". Some compilers and compiler
 *  settings will allow this construct but it is not good practice to write
 *  code like this so you should consider rewriting the code anyway.
 *
 */

#ifndef DEBUG_HPP_
#define DEBUG_HPP_

// Define the serial channel for your debug prints
#define lSer      Serial

//#define DEBUG
#ifdef DEBUG
#define PRINT(x)  lSer.print(x)
#define PRINTLN(x) lSer.println(x)
#define WRITE(x)   lSer.write(x)
#else
#define PRINT(x)
#define PRINTLN(x)
#define WRITE(x)
#endif

#endif /* DEBUG_HPP_ */
