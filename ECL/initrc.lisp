(format t "loading initrc.lisp~%")

;; Test function to call in emacs after starting slime-connect
(defun testecl()
  (format t "Thank <insert deity or expletive here>! it actually works!!!"))

(let ((quicklisp-init  "~/quicklisp/setup.lisp"))
  (when (probe-file quicklisp-init)
    (load quicklisp-init)))

(format t "ASDF Version ~a~%" (asdf:asdf-version))

(ql:quickload :swank)

;;Starting a swank server
(defun start-server ()
  (let ((swank::*loopback-interface* "127.0.0.1"))
    (swank:create-server :port 4005 :style :spawn :dont-close t)))

(start-server)

(format t "I've run the contents of initrc.lisp~%")


