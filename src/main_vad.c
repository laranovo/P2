#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sndfile.h>

#include "vad.h"
#include "vad_docopt.h"

#define DEBUG_VAD 0x1

int main(int argc, char *argv[]) {
  int verbose = 0; /* To show internal state of vad: verbose = DEBUG_VAD; */

  SNDFILE *sndfile_in, *sndfile_out = 0;
  SF_INFO sf_info;
  FILE *vadfile;
  int n_read = 0, i;

  VAD_DATA *vad_data;
  VAD_STATE state, last_state;

  float *buffer, *buffer_zeros;
  int frame_size;         /* in samples */
  float frame_duration;   /* in seconds */
  unsigned int t, last_t; /* in frames */

  char	*input_wav, *output_vad, *output_wav;

 //**************MIS VARIABLES**************
  int total_voice=0, total_silence=0, cont=11, size=11, estados[size];
  float pot_acum=0;

  for  (i=0; i<size; i++) 
    estados[i]=0;

  i=0;
//******************************************

  DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ "2.0");

  verbose    = args.verbose ? DEBUG_VAD : 0;
  input_wav  = args.input_wav;
  output_vad = args.output_vad;
  output_wav = args.output_wav;

  if (input_wav == 0 || output_vad == 0) {
    fprintf(stderr, "%s\n", args.usage_pattern);
    return -1;
  }

/* Open input sound file */
  if ((sndfile_in = sf_open(input_wav, SFM_READ, &sf_info)) == 0) {
    fprintf(stderr, "Error opening input file %s (%s)\n", input_wav, strerror(errno));
    return -1;
  }

  if (sf_info.channels != 1) {
    fprintf(stderr, "Error: the input file has to be mono: %s\n", input_wav);
    return -2;
  }

  /* Open vad file */
  if ((vadfile = fopen(output_vad, "wt")) == 0) {
    fprintf(stderr, "Error opening output vad file %s (%s)\n", output_vad, strerror(errno));
    return -1;
  }

  /* Open output sound file, with same format, channels, etc. than input */
  if (output_wav) {
    if ((sndfile_out = sf_open(output_wav, SFM_WRITE, &sf_info)) == 0) {
      fprintf(stderr, "Error opening output wav file %s (%s)\n", output_wav, strerror(errno));
      return -1;
    }
  }
  vad_data = vad_open(sf_info.samplerate);
  /* Allocate memory for buffers */
  frame_size   = vad_frame_size(vad_data);
  buffer       = (float *) malloc(frame_size * sizeof(float));
  buffer_zeros = (float *) malloc(frame_size * sizeof(float));

  for (i=0; i< frame_size; ++i) 
    buffer_zeros[i] = 0.0F;

  frame_duration = (float) frame_size/ (float) sf_info.samplerate;
  last_state = ST_UNDEF;


  for (t = last_t = 0; ; t++) { /* For each frame ... */
    /* End loop when file has finished (or there is an error) */
    if  ((n_read = sf_read_float(sndfile_in, buffer, frame_size)) != frame_size) break;

    if (sndfile_out != 0) {
      /* TODO: copy all the samples into sndfile_out */
    }

    state = vad(vad_data, buffer);
    //*******************
    /*LÓGICA DEL CÓDIGO
    En este código lo que haremos a grandes rasgos es lo siguiente:
    - Usaremos un vector que almacene los últimos estados y se vaya actualizando
    - Si nos encontramos con un fragmento MAYBE, miraremos el buffer de estados y 
    decidiremos si estamos en un fragmento de voz o silencio viendo la resta entre la cantidad
    total de estos estados de cada tipo en el vector. Si se diera el caso que la cantidad entre
    silencio y voz es la misma, entonces tomaremos como referencia la potencia. Iremos acumulando
    también la potencia de las últimas muestras y si no se supera cierto umbral determinaremos
    que estamos en un estado de silencio.
    */
    estados[size-t % size-1] = state;
    if(state==ST_MAYBE_VOICE || state==ST_MAYBE_SILENCE){
      cont=size;
      while(estados[cont]!=0 || cont==0){
        if(estados[cont]==ST_VOICE){
          total_voice+=1;
        }
        else if(estados[cont]==ST_SILENCE){
          total_silence +=1;
        } 
        pot_acum+=buffer[cont]*buffer[cont];
        cont-=1;       
      }
      cont=0;
      if(total_voice-total_silence > 1) 
        state=ST_VOICE;
      else if(total_silence-total_voice > 1 ) 
        state=ST_SILENCE;
      else{
        if (pot_acum>0.0075){
          state=ST_VOICE;
        }else{
          state=ST_SILENCE;
        }
      }
      pot_acum=0;    
      total_voice=0; 
      total_silence=0;  
    }

    if (verbose & DEBUG_VAD) vad_show_state(vad_data, stdout);

    /* TODO: print only SILENCE and VOICE labels */
    /* As it is, it prints UNDEF segments but is should be merge to the proper value */


    if (state != last_state) {
      if (t != last_t)
        fprintf(vadfile, "%.5f\t%.5f\t%s\n", last_t * frame_duration, t * frame_duration, state2str(last_state));
      last_state = state;
      last_t = t;
    }

    if (sndfile_out != 0) {
      /* TODO: go back and write zeros in silence segments */
    }
  }

  state = vad_close(vad_data);
  /* TODO: what do you want to print, for last frames? */
  if (t != last_t)
    fprintf(vadfile, "%.5f\t%.5f\t%s\n", last_t * frame_duration, t * frame_duration + n_read / (float) sf_info.samplerate, state2str(state));

  /* clean up: free memory, close open files */

  free(buffer);
  free(buffer_zeros);
  sf_close(sndfile_in);
  fclose(vadfile);
  if (sndfile_out) sf_close(sndfile_out);
  return 0;
}
