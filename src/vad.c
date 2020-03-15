#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "pav_analysis.h"
#include "vad.h"

const float FRAME_TIME = 10.0F; /* in ms. */
const int NINIT=9;
const float ALPHA1=2.32;
const float ALPHA2=6.28;

/*
As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
only this labels are needed. You need to add all labels, in case
you want to print the internal state in string format */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
  float k0;
  float k1;
  float k2;
  int time_vs;
  int total_silence;
  int total_voice;
} Features;

/*
* TODO: Delete and use your own features!
*/

Features compute_features(const float *x, int N) {
  /*
  * Input: x[i] : i=0 .... N-1
  * Ouput: computed features
  */
  /*
  * DELETE and include a call to your own functions
  *
  * For the moment, compute random value between 0 and 1
  */
  Features feat;

  int i=0;
  double sumatory=0;
  for(i=0; i<N; i++){
    sumatory+=x[i]*x[i];
  }
  feat.p=10*log10(sumatory/N);

  return feat;
}

/*
* TODO: Init the values of vad_data
*/

VAD_DATA * vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  vad_data->k0=0;
  vad_data->total_silence=0;
  vad_data->total_voice=0;
  vad_data->time_vs=20;
  vad_data->lmin_v=0;
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /*
  * TODO: decide what to do with the last undecided frames
  */
  VAD_STATE state = ST_SILENCE;

  
  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/*
* TODO: Implement the Voice Activity Detection
* using a Finite State Automata
*/

VAD_STATE vad(VAD_DATA *vad_data, float *x) {

  /* TODO: You can change this, using your own features,
  * program finite state automaton, define conditions, etc.
  */

  Features f = compute_features(x, vad_data->frame_length);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state) {

    case ST_INIT:
    if(vad_data->total_silence < NINIT){
      vad_data->k0+=pow(10,f.p/10);
      vad_data->state=ST_INIT;
      vad_data->total_silence+=1;
    }
    else{
      vad_data->total_silence=0;
      vad_data->k0=10*log10(vad_data->k0/NINIT);
      vad_data->k1=vad_data->k0+ALPHA1;
      vad_data->k2=vad_data->k1+ALPHA2;
      vad_data->state=ST_SILENCE;
    }
    break;

    case ST_VOICE: 
      if(vad_data->k2 <= f.p){
        vad_data->state=ST_VOICE;
      }else{
        vad_data->state=ST_MAYBE_SILENCE;
        vad_data->total_silence=0;
      }
    break;    
    
    case ST_SILENCE:
      if(vad_data->k1 >= f.p){
        vad_data->state=ST_SILENCE;
      }else{
        vad_data->state=ST_MAYBE_VOICE;
        vad_data->total_voice=0;
      }
    break;

    case ST_MAYBE_VOICE:
        if(f.p >= vad_data->k2){
          vad_data->state=ST_VOICE;
        }else if(f.p <= vad_data->k1){
          vad_data->state=ST_SILENCE;
        }else{
          vad_data->state=ST_MAYBE_VOICE;
          vad_data->total_voice+=1;
        }
    break;

    case ST_MAYBE_SILENCE:
      if(vad_data->total_silence < vad_data->time_vs){
        if(f.p >= vad_data->k2){
          vad_data->state=ST_VOICE;
        }else{
          vad_data->state=ST_MAYBE_SILENCE;
          vad_data->total_silence+=1;
        }
      }else{
        if(f.p >= vad_data->k2){
          vad_data->state=ST_VOICE;
        }else if(f.p <= vad_data->k1){
          vad_data->state=ST_SILENCE;
        }else{
          vad_data->state=ST_MAYBE_SILENCE;
          vad_data->total_silence+=1;
        }
      }
    break;

    case ST_UNDEF:
    break;
  }

  /*
  if (vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE)
    return vad_data->state;
  else
    return ST_UNDEF;
  */

  //Suponemos que cualquier muestra de audio empezará con un silencio
  if(vad_data->state == ST_INIT){
    return ST_SILENCE;
  }else{
    return vad_data->state;
  }
  
}

  void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
    fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
  }