;; This example shows how to design a simple low-pass filter.  To run
;; this example, type `gmeteor lowpass.scm' at the command-line
;; prompt.

(define maxFreq 44100)
(define slop .2)

(display "lowpass limit: ")
(define low (read))

;; set verbose mode on.
(verbose #t)

;; our filter has cosine (i.e., even) symmetry, and its length is 101
(cosine-symmetry)
(filter-length 101)

(sampling-frequency maxFreq)

(limit-= (band 0 low) 1 )
(limit-= (band (* low (+ 1 slop)) (/ maxFreq 2)) 0 )

(define name (number->string low))
(output-file (string-append "lo" name))
(plot-file "p")

(go)
