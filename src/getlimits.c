/****************************************************************************
 * Copyright (C) 2015 by Simone Bnà(a), Sandro Manservisi(a),               *
 * Ruben Scardovelli(a), Philip Yecko(b) and Stephane Zaleski(c,d)          *
 * (a) DIN–Lab. di Montecuccolino, Università di Bologna,                   *
 *     Via dei Colli 16, 40136 Bologna, Italy                               *
 * (b) Physics Department, Cooper Union, New York, NY, USA                  *
 * (c) Sorbonne Universités, UPMC Univ Paris 06, UMR 7190,                  *
 *     Institut Jean Le Rond d’Alembert, F-75005, Paris, France             *
 * (d) CNRS, UMR 7190, Institut Jean Le Rond d’Alembert, F-75005,           *
 *     Paris, France                                                        *
 *                                                                          *
 * You should have received a copy of the CPC license along with Vofi.      *
 * If not, see http://cpc.cs.qub.ac.uk/licence/licence.html.                *
 *                                                                          *
 * e-mail: ruben.scardovelli@unibo.it                                       *
 *                                                                          *
 ****************************************************************************/

/**
 * @file getlimits.c
 * @authors Simone Bnà, Sandro Manservisi, Ruben Scardovelli, 
 *          Philip Yecko and Stephane Zaleski 
 * @date  12 November 2015
 * @brief It subdivides the side along the secondary or tertiary
 *        direction to define rectangles or rectangular hexahedra 
 *        with or without the interface. 
 */

#include "vofi_stddecl.h"

/* -------------------------------------------------------------------------- *
 * DESCRIPTION:                                                               *
 * subdivide the side along the secondary/tertiary (2/3) direction to define  *
 * rectangles/rectangular hexahedra with or without the interface             *
 * INPUT: pointer to the implicit function, starting point x0, primary,       *
 * secondary, tertiary directions pdir, sdir, tdir, grid spacing h0,          *
 * subdivision direction stdir (2/3)                                          *
 * OUTPUT: nsub: total number of subdivisions; array lim_intg: start/end of   *
 * each subdivision (lim_intg[0] = 0, lim_intg[nsub] = h0)                    *
 * -------------------------------------------------------------------------- */

int vofi_get_limits(integrand impl_func,void *userdata,vofi_creal x0[],vofi_real lim_intg[],vofi_creal
                    pdir[],vofi_creal sdir[],vofi_creal tdir[],vofi_creal h0,vofi_cint stdir)
{
  int i,j,k,iv,nsub,nvp,nvn;
  vofi_real fv[NVER],x1[NDIM],x2[NDIM],fe[NEND],ds,ls;
  chk_data fvga; 
  min_data xfsa;
  
  lim_intg[0] = 0.;
  nsub = 1;  
  if (stdir == 2) {                     /* get the internal limits along sdir */
    for (j=0;j<2;j++) {                                          /* two sides */
      for (i=0;i<NDIM;i++) {
	x1[i] = x0[i] + j*pdir[i]*h0;
	x2[i] = x1[i] + sdir[i]*h0;
      }
      fe[0] = impl_func(userdata,x1);
      fe[1] = impl_func(userdata,x2);
      vofi_get_side_intersections(impl_func,userdata,fe,x1,lim_intg,sdir,h0,&nsub);
    }
  }
  else {                                /* get the external limits along tdir */
    for (k=0;k<2;k++) {                                         /* two planes */
      /* DEBUG 1 */

      nvp = nvn = iv = 0;
      for (j=0;j<2;j++) {                                        /* two sides */
	/* DEBUG 2 */

	for (i=0;i<NDIM;i++) { 
	  x1[i] = x0[i] + k*pdir[i]*h0+j*sdir[i]*h0;
	  x2[i] = x1[i] + tdir[i]*h0;
	}
	fe[0] = impl_func(userdata,x1);
	fv[iv++] = fe[0];
	fe[1] = impl_func(userdata,x2);
	fv[iv++] = fe[1];
	if (fe[0]*fe[1] >= 0.0) {
	  if ((fe[0]+fe[1]) > 0.)
	    nvp += 2;
	  else
	    nvn += 2;
	}
	vofi_get_side_intersections(impl_func,userdata,fe,x1,lim_intg,tdir,h0,&nsub);
	/* DEBUG 3 */

      }
      if (nvp == 4 || nvn == 4) {         /* get the extra limits in the face */
	/* DEBUG 4 */

	xfsa.iat = 0;
	for (i=0;i<NDIM;i++) 
	  x1[i] = x0[i] + k*pdir[i]*h0;
	fvga = vofi_check_face_consistency(impl_func,userdata,fv,x1,sdir,tdir,h0); 
	if (fvga. iat != 0)
	  xfsa = vofi_get_face_min(impl_func,userdata,x1,sdir,tdir,fvga,h0);
	if (xfsa.iat != 0)
	  vofi_get_face_intersections(impl_func,userdata,xfsa,x1,lim_intg,sdir,tdir,h0,
                                      &nsub);
	/* DEBUG 5 */

      } 
    }
  }
  lim_intg[nsub] = h0;

  /* DEBUG 6 */

  for (j=2;j<nsub;j++) {                         /* order limits from 0 to h0 */
    ls = lim_intg[j];
    i = j-1;
    while (i > 0 && lim_intg[i] > ls) {
      lim_intg[i+1] = lim_intg[i];
      i--;
    }
    lim_intg[i+1] = ls;
  }

  i = 0;
  while (i<nsub) {                             /*remove zero-length intervals */
    ds = lim_intg[i+1] - lim_intg[i];
    if (ds < EPS_R) {
      for (j=i;j<nsub;j++)
	lim_intg[j] = lim_intg[j+1];
      nsub--;
    }
    i++;
  }
  lim_intg[0] = 0.;                                        /* just for safety */
  lim_intg[nsub] = h0;
  /* DEBUG 7 */

  return nsub;    
}
