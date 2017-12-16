//Author: Michael Raines
//Purpose: rrastrip will strip the white noise from the beginning
//and end of a rra file.

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"

char* PROGRAM_NAME = "rrastrip";
char* PROGRAM_VERSION = "0.01";
const int WINDOW_SIZE = 30;
int inPlace = 0;
int AMP_MIN = 200;
int FINISH_AMP_MIN = 100;
int badFile = 0;

void rraStrip( char* file, FILE *in, FILE *out ){
        int startFound = 0;
        int i;
        int seriesCount = 0;
        int ramped = 0;
        int rampCount = 0;
        int rampInc = 1/WINDOW_SIZE;
        RRA *track;
        if( in == NULL ){
                printf("Unable to open file %s\n", file);
                badFile = 1;
                return;
        }
        track = newRRAHeader();
        readRRAHeader( in, track, 0 );
        if( track == NULL ){
                printf("%s is not a well formed RRA file\n",file);
                badFile = 1;
                return;
        }
        if( track->channels == 2 ){
                printf("%s is a stereo RRA file, please split with rramono first.\n", file);
                badFile = 1;
                return;
        }
        for( i = 0; !feof( in ); i++){
                int amp = readRRAAmplitude(in,out,track->bitsPerSample,0);
                if( !startFound ){
                        if( amp >= AMP_MIN || amp <= ( -1 * AMP_MIN ) ){
                                seriesCount++;
                                if(seriesCount >= WINDOW_SIZE ){
                                        startFound = 1;
                                        i -= WINDOW_SIZE;
                                        track->samples = track->samples - (i + 1);
                                        writeRRAHeader( out, track, "modifiedBy: rrastrip", 0 );
                                }
                        }else{
                                
                                seriesCount = 0;
                        }
                }else{
                        if( !ramped ){
                                amp = amp * (rampCount * rampInc);
                                rampCount++;
                                if( rampCount >= WINDOW_SIZE ){
                                        ramped = 1;
                                }
                        }
                        fprintf( out, "%d\n", amp );
                }
        }
        if( !startFound ){
                printf("Found no usable amplitudes in %s. Leaving unaltered.", file );
                badFile = 1;
        }else{
                printf("%s stripped sucessfully.\n", file );
        }

        fclose( in );
        fclose( out );
}

static int processOptions( int argc, char **argv ){
        int argIndex;
        int argUsed;
        int seperateArg;
        char* arg;

        argIndex = 1;

        while( argIndex < argc && *argv[argIndex] == '-' ){
                seperateArg = 0;
                argUsed = 0;

                if( argv[argIndex][2] == '\0' ){
                        arg = argv[argIndex+1];
                        seperateArg = 1;
                }else{
                        arg = argv[argIndex]+2;
                }
                
                switch( argv[argIndex][1] ){
                        case 'v':
                                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                                exit(0);
                                break;
                        case 'i':
                                inPlace = 1;
                                break;
                        case 'a':
                                AMP_MIN = atoi(arg);
                                argUsed = 1;
                                break;
                        default:
                                Fatal("option %s not understood\n", argv[argIndex]);
                }
                if( seperateArg && argUsed ){
                        ++argIndex;
                }
                ++argIndex;
        }
        return argIndex;
}

int main( int argc, char* argv[] ){
        int i;
        int argIndex = processOptions( argc, argv );
        FILE *in, *out;
        char* inFile;
        char buffer[4096];
        /*for( i = argIndex; i < argc; i++){
                in = OpenFile(argv[i], "r");
                sprintf(buffer, "%s%s", "stripped_", argv[i]);
                out = OpenFile(buffer, "w");
                rraStrip( argv[i], in, out );
        }*/
        if( argc-argIndex == 0){
                in = stdin;
                out = stdout;
                inFile = "stdin";
        }else 
        if(argc - argIndex == 1 && !inPlace){
                in = OpenFile(argv[argIndex],"r");
                out = stdout;
                inFile = argv[argIndex];
        }else if(argc - argIndex == 2 && !inPlace){
                in = OpenFile(argv[argIndex],"r");
                out = OpenFile(argv[argIndex+1],"w");
                inFile = argv[argIndex];
        }else if( argc - argIndex >= 1 && inPlace){
                for( i = argIndex; i < argc; i++){
                        sprintf(buffer,"mv %s %s%s",argv[i], argv[i],".temp");
                        system(buffer);
                        sprintf(buffer,"%s%s",argv[i],".temp");
                        in = OpenFile(buffer,"r");
                        out = OpenFile(argv[i],"w");
                        inFile = argv[i];
                        rraStrip( inFile, in, out );
                        if( !badFile ){
                                sprintf(buffer,"rm %s%s",argv[i],".temp");
                                system(buffer);
                        }else{
                                badFile = 0;
                                sprintf(buffer,"mv %s%s %s",argv[i],".temp", argv[i]);
                                printf("Running command: %s\n",buffer);
                                system(buffer);
                        }
                }
                return 0;
        }else{
                printf("usage: rrastrip [-aN ] [ -i ] [<input rra>] [<output rra>]\n");
                exit(0);
        }
        rraStrip( inFile, in, out );
        return 0;
}
