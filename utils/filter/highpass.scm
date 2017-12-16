(verbose #t)

(define maxFreq 44100)
(define slop .2)
(display "highpass limit: ")
(define high (read))

(cosine-symmetry)
(filter-length 331)

(sampling-frequency maxFreq)

(limit-= (band 0 high) 0 )
(limit-= (band (* high (+ 1 slop)) (/ maxFreq 2)) 1 )

(define name (number->string high))
(output-file (string-append "hi" name))
(plot-file "p")

(go)
