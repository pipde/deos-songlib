import jm.JMC;
import jm.music.data.*;
import jm.util.*;

/**
   * A short example which generates a one octave c chromatic scale
   * and writes to a standard MIDI file called ChromaticScale.mid
   * @author Andrew Sorensen and Andrew Brown
   *
   * modified by Joun C. Lusth, 2006
   *
   */

public final class Pitch implements JMC
    {
    public static void main(String[] args)
        {
        if (args.length != 3)
            {
            System.out.println("usage: java Pitch <inst> <note number>"
                    + " <duration>");
            System.exit(0);
            }

        int channel = 0;
        int inst = Integer.parseInt(args[0]);
        int note = Integer.parseInt(args[1]);
        int duration = Integer.parseInt(args[2]);

        //set up parts and scores
        
        Score score = new Score("pitch");
        Part part = new Part(inst,channel);
        Phrase phrase = new Phrase("note", 0.0); 
        
        // create the notes

        phrase.addNote(new Note(note,duration));

        // add phrase to part
        
        part.addPhrase(phrase);
        
        // add part to score
        
        score.addPart(part);
         
        //write the MIDI file 

        Write.midi(score, "pitch_" + note + ".mid");
        }
    }
