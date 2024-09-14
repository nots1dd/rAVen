/*
 * @UNDERSTANDING Fourier Transform
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <complex.h>

#define pi 3.14159265358979323846f

/************************************************************
 *  ALTERNATIVE to PI 
 *
 *  pi = atanf(1,1)*4;
 *
 *  Here, (1,1) is a vector with angle 45 deg
 *  Just multiply by 4 to give very accurate version of pi
 *
 ************************************************************/

/************************************
 * @DFT - Regular Fourier Transform
 ************************************/

void dft(float in[], float out[], size_t n) {
  for (size_t f=0;f<n;f++) {
    out[f] = 0;
    for (size_t i=0;i<n;i++) {
      float t = (float)i/n;
      //cexp(I*2*pi*f*t); // cexp represents the complex exponential function
      out[f] += in[i]*cexp(I*2*pi*f*t);
    }
  }
}

/**************************************
 * @FFT - Fast Fourier Transform
 * (DIVIDE and CONQUER)
 **************************************/

void fft(float in[], size_t stride, float complex out[], size_t n) {
  /*
   * @STRIDE => Similar to python's index slicing [Ex: in[0::2]]
   */

  assert(n > 0);
  
  /*
   * $BASE CASE
   */
  if (n == 1) {
    out[0] = in[0];
    return;
  }

  fft(in, stride*2, out, n/2);
  fft(in + stride, stride*2, out + n/2, n/2);

  for (size_t k=0;k<n/2;k++) {
    float t = (float)k/n;
    float complex v = cexp(-2*I*pi*t)*out[k+n/2];
    float complex e = out[k];
    out[k]     = e + v;
    out[k+n/2] = e - v;
  }
}

int main() {
  size_t n = 4096;
  float in[n]; // samples
  /******************************************************************
   *
   * @NOTE 
   *
   * USING EULER'S FORUMLA:
   *
   * e^(ix) = cos(x) + isin(x)
   *
   * We can keep track of the values of sin and cosine simultaneously
   *
   *******************************************************************/
  float complex out[n]; // output frequencies

  for (size_t i=0;i<n;i++) {
    float t = (float)i/n;
    in[i] = cosf(2*pi*t*1) + sinf(2*pi*t*2);
  }
  /*****************************
   *
   * @UNMIXING SIGNALS
   *
   *****************************/
  
  /*for (size_t f = 0; f < n/2; f++) {*/
  /*  out[f] = 0;*/
  /*  out[f+n/2] = 0; // READ @UNDERSTANDING FAST FOURIER TRANSFORM for more*/
  /**/
  /*  // $EVEN: */
  /*  for (size_t i=0;i<n;i+=2) {*/
  /*    float t = (float)i/n;*/
  /*    float complex val = in[i]*cexp(I*2*pi*t*f); */
  /*    out[f] += val;*/
  /*    out[f+n/2] += val;*/
  /*  }*/
  /*  // $ODD*/
  /*  for (size_t i=1;i<n;i+=2) {*/
  /*    float t = (float)i/n;*/
  /*    float complex val = in[i]*cexp(I*2*pi*t*f); */
  /*    out[f] += val;*/
  /*    out[f+n/2] -= val;*/
  /*  }*/
  /*} */
  
  // DFT 
  // dft(in, out, n)

  fft(in, 1, out, n);
  
  for (size_t f=0;f<n;f++) {
    // cos(x) + isin(x) ==> REAL PART: COS(X) ;; IMAG PART: SIN(X)
    printf("%0zu: %.2f, %.2f\n", f, creal(out[f]), cimag(out[f]));
  }

  return 0;
}

/*********************************************
 *
 * @TIME COMPLEXITY
 *
 * This is O(n^2) which is not very efficient
 *
 * Fast Fourier Transform is O(nlogn)
 * 
 *********************************************/



/************************************************************************************************************************ 
 *
 * @UNDERSTANDING FAST FOURIER TRANSFORM 
 *
 * $OBSERVATION:
 *
 * Upon printing all sinf frequencies we notice that
 * there are a lot symmetrically identical values.
 *
 * FFT exploits this symmetry to make sure that we do not have to 
 * iterate over every sample which would lead to a O(n^2) complexity
 *
 * FFT has managed to cut down such computations by HALF
 *
 * $STEPS FOR FFT 
 *
 * 1. Split the 'n' samples in half [into left-half and right-half]
 * 2. Find the EVEN FREQUENCIES of some 'f' in left-half
 * 3. Compare the observe the EVEN FREQUENCIES of f+n/2 as well {THEY WILL BE THE SAME}
 *
 * Example:
 * [n = 16] ==> [+ 1 + + + + + +] | [+ 9 + + + + + +] 
 *    size_t f = 1; // LEFT HALF
 *    // Find the EVEN frequencies sinf values of this f
 *    f += n/2;
 *    // Find the EVEN frequencies of this new f 
 *
 *    THEY WILL BE THE SAME 
 *    SO WE FIND THAT THE EVEN frequencies 1 and 9 ARE THE SAME !!
 *
 *  Similarly, 
 *
 *  FOR ODD FREQUENCIES, 
 *    LEFT-HALF = -(RIGHT-HALF)
 *
 *  $CODE IMPLEMENTATION
 *
 *  O(n^2) [Fourier Transform]:
 * ---------------------------------------------------------------------------------------
 *      for (size_t f=0;f<n;f++) {
 *         out[f] = 0;
 *         for (size_t i=0;i<n;i++) {
 *           float t = (float)i/n;
 *           //cexp(I*2*pi*f*t); // cexp represents the complex exponential function
 *           out[f] += in[i]*cexp(I*2*pi*f*t);
 *         }
 *       } 
 * ---------------------------------------------------------------------------------------
 *
 *  Basic FFT [Fast Fourier Transform]: {NON-RECURSIVE}
 * ---------------------------------------------------------------------------------------
 *     for (size_t f = 0; f < n/2; f++) {
 *       out[f] = 0;
 *       out[f+n/2] = 0; // READ @UNDERSTANDING FAST FOURIER TRANSFORM for more
 *
 *       // $EVEN: 
 *       for (size_t i=0;i<n;i+=2) {
 *         float t = (float)i/n;
 *         float complex val = in[in]*exp(I*2*pi*t*f); 
 *         out[f] += val;
 *         out[f+n/2] += val;
 *       }
 *       // $ODD
 *       for (size_t i=1;i<n;i+=2) {
 *         float t = (float)i/n;
 *         float complex val = in[in]*exp(I*2*pi*t*f); 
 *         out[f] += val;
 *         out[f+n/2] -= val;
 *       }
 *     } 
 * ---------------------------------------------------------------------------------------
 *
 *  $AFTERTHOUGHT
 *
 *  Performing FFT on the EVEN or ODD halfs again will lead to another symmetry 
 *
 *  So we need to make the basic FFT RECURSIVE to implement DIVIDE and CONQUER
 *
 *  Thus FFT employs a form of DIVIDE and CONQUER method, leading to O(nlogn)
 *
 *  $LIMITATIONS
 *
 *  FFT requires the sample size to be 2^n always (APPARENTLY**)
 *
 *  $NOTE 
 *
 *  For full RECURSIVE method of FFT, check @void fft(float in[], size_t stride, float complex out[], size_t n)
 *
 ************************************************************************************************************************/ 
