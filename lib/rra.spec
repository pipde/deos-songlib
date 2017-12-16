Introduction:

    The purpose of this document is to specify the audio format
    used by songlib. The (Readily-Readable Audio) format is designed
    to be easily viewed (and debugged) and extensible. Originally
    suggested by Ian Taylor for use in a High School Programming
    Contest, it has been adopted as the fundamental format by songlib
    to allow for less-experienced programs to improve and extend 
    the library.

    An RRA file is composed of two sections, a header section and
    a data section.

Grammar:

    An RRA file has the following grammar. Non-terminals are in
    lowercase and terminals are in UPPERCASE. An RRA file begins
    with the seven ASCII characters RRAUDIO.

	file : "RRAUDIO" optTags "%%" data

	optTags : *empty*
		| tag optTags

	tag : attribute COLON value

	value : TOKEN | STRING

	data : INTEGER
         | REAL
	     | INTEGER data
	     | REAL data

    An RRA file is free-format and whitespace delimited.
    Comments begin with a bang (!) and continue until the
    end of line.  Here is an example file:

        RRAUDIO
        ! Hohner Pocket Pal Harmonica
        ! D#
        ! recorded by Becky Smith, February 2011
        sampleRate: 44100
        bitsPerSample: 24
        channels: 1
        multiNote: 1203928
        multiNote: 2593821
        # amplitude data begins
        %%
        0
        0
        1
        -2
        ...

    Note that blank lines, whitespace before a colon, and
    tags spanning multiple lines are all permissible. Comments
    may appear in the data section as well.

Required tags:

    No tags are required but all applications must support the following tags:

        channels: <positive integer>
        sampleRate: <positive integer>
        bitsPerSample: <postive integer>
        samples: <a non-negative integer>

    If not present, the following values are assumed for the
    the supported attributes:

        channels: 1
        sampleRate: 44100
        bitsPerSample: 16
        samples: 0

    The value for samples indicates the number of samples
    in one channel. To find the total number of samples
    across all channels, one would multiply samples by
    channels.

    A zero value for the samples attribute indicates that
    the number of samples is not specified and an application
    should continue to read data until the end-of-file. If at
    all possible, an application should set the size attribute
    accurately. It is up to the application to decide how
    to handle a size value that doesn't match the number
    actual amount of amplitude data. At a minimum, a warning
    message should be generated.

Duplicate tags:

    The meaning of duplicate tags is not specified. Three reasonable
    alternatives come to mind:

        * all subsequent tags are ignored
	
        * all previous tags are ignored
	
        * the attribute is variadic and the values accumulate
	
    An example of the latter might be a multiple multiNote tags.
    Multiple instances of a note might be contained in the same RRA
    file, where the value of each multiNote attribute indicates the
    offset where the another note can be found.

    For the previous cases, duplicate warning messages should be
    generated.
	
Unknown tags:

    An application should handle unknown tags gracefully. At a mininum,
    an unknown tag should be ignored but a warning message should
    be generated. If the application is producing an RRA file, the
    unkown tag should appear in the resulting RRA file.

Multiple channels:

    The number of channels specifies how many different audio streams are
    combined to make the whole. For example, monaural audio (channels: 1)
    has a single channel.  Stereo audio (channels: 2) has two channels
    (left and right), while quadraphonic audio (channels: 4) has four
    channels, and so on. The amplitude data for multiple channels
    are interleaved. For stereo, the amplitude data/samples looks like this:

     Amplitude_1_Channel_0
     Amplitude_1_Channel_1
     Amplitude_2_Channel_0
     Amplitude_2_Channel_1
     ...
     Amplitude_N_Channel_0
     Amplitude_N_Channel_1

    Channels are number starting at zero.

    A warning message should be generated if the total number of
    samples is not evenly divisible by the number of channels.

Comments:

    Comments are generally intended to provide dynamic input to an
    application. For example, the comment:

        !amplify: +20%

    might instruct an application to boost the volume of the RRA 20 percent.
    If an application encounters a comment intended for some other 
    application, the comment should be reproduced on the output.

Warning messages:

    An application should allow the user to turn off warning messages.
