import jm.JMC;
import jm.music.data.*;
import jm.util.*;

/**
   * A short example which generates a one octave c chromatic scale
   * and writes to a standard MIDI file called ChromaticScale.mid
   * @author Andrew Sorensen and Andrew Brown
   *
   * modified by John C. Lusth, 2006
   *
   */

public final class Scale implements JMC
    {
    public static void main(String[] args)
        {
        int argIndex = 0;
        int channel = 0;
        Score score = new Score("scale");

        while (args[argIndex].charAt(0) == '-')
             {
             if (args[argIndex].equals("-c"))
                {
                channel = Integer.parseInt(args[argIndex+1]);
                System.out.println("switching channel to " + channel);
                argIndex += 2;
                }
            else
                {
                System.out.println("option " + args[argIndex] + 
                    " not understood");
                argIndex += 1;
                }
            }
        
        // handle the regular arguments
        
        if (args.length == 0)
            {
            System.out.println("usage: java Scale <inst> <octave> <count>"
                    + " <duration>");
            System.exit(0);
            }
        int inst = Integer.parseInt(args[argIndex]);
        System.out.println("instrument is " + inst);
        int octave = args.length - argIndex > 1 ?
            Integer.parseInt(args[argIndex+1]) : 3;
        int duration = args.length - argIndex > 3 ?
            Integer.parseInt(args[argIndex+2]) : 2;

        //set up parts and scores
        
        Part part = new Part(inst,channel);
        Phrase phrase = new Phrase("upward scale", 0.0); 
        
        // create the notes

        phrase.addNote(new Note(octave * 12 + 0,duration));
        phrase.addNote(new Note(octave * 12 + 2,duration));
        phrase.addNote(new Note(octave * 12 + 4,duration));
        phrase.addNote(new Note(octave * 12 + 5,duration));
        phrase.addNote(new Note(octave * 12 + 7,duration));
        phrase.addNote(new Note(octave * 12 + 9,duration));
        phrase.addNote(new Note(octave * 12 + 11,duration));
        phrase.addNote(new Note(octave * 12 + 12,duration));

        // add phrase to part
        
        part.addPhrase(phrase);
        
        // add part to score
        
        score.addPart(part);
         
        //write the MIDI file 

        Write.midi(score, "scale_" + inst + ".mid");
        }
    }
