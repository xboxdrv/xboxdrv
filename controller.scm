;; Trouble: How to handle two output, like for example an axis rotator?
;;
;; (axis x) -> o+-------+o -> (axis-y)
;;              |AxisRot|
;; (axis y) -> o+-------+o -> (axis-x)
;;
;; Just make two?
;; --------------
;; (axis x) -> o+-------+
;;              |AxisRot|o -> (axis-x)
;; (axis y) -> o+-------+
;;
;; (axis x) -> o+-------+
;;              |AxisRot|o -> (axis-y)
;; (axis y) -> o+-------+

;; forward config can't do modifier, so we do it backward

(uinput-device
 (name "XBox360 Controller")
 (led  status)

 (map
  (abs (id "ABS_X")
       (min      0)
       (max    500)
       (clamp   #t)
       (stretch #t)
       (input   (xbox360-button (id "X"))))
  
  ;; ---------------------------------------

  (abs (id "ABS_X")
       (input (axis-rotate 
               (angle 45)
               (input-x (button (id "X")))
               (input-y (button (id "Y"))))))

  (abs (id "ABS_Y")
       (input (axis-rotate 
               (angle 45)
               (input-x (button (id "Y")))
               (input-y (button (id "X"))))))

  ;; ---------------------------------------

  (abs (id "ABS_THROTTLE")
       (input (throttle 
               (speed 10)
               (input (xbox360-button (id "X"))))))

  ;; ---------------------------------------
  
  (key (id    "KEY_UP") 
       (input (axis-button 
               (threshold 25)
               (axis      (axis (id "X"))))))

  ;; ---------------------------------------

  (rel (id "REL_X")
       (input (axis (id "X"))))

  (rel (id "REL_Y")  
       (input (invert (axis (id "Y")))))

  ;; ---------------------------------------

  (key (id    "KEY_UP") 
       (input (button (id "DPAD_UP"))))

  (key (id    "KEY_DOWN") 
       (input (button (id "DPAD_DOWN"))))

  (key (id    "KEY_LEFT") 
       (input (button (id "DPAD_LEFT"))))

  (key (id    "KEY_RIGHT") 
       (input (button (id "DPAD_RIGHT"))))

  ;; ---------------------------------------

  (key (id "KEY_A")
       (trigger (button-combo (xbox360-button (id "A"))
                              (xbox360-button (id "X")))))
  
  (command ;; fork/exec
   (exec  "/usr/bin/foobar" "file" "foo")
   (call  "/usr/bin/ls -l")
   (input (button (id "MODE"))))

  (switch ;; Shift to another configuration file
   (conf "foobar.scm")
   (led  "blink4")
   (input (shift (shift   (button (id "MODE")))
                 (trigger (button (id "A"))))))

  ))

;; Alternative Short Hand Syntax (make named arguments optional):
(abs "ABS_Y" "BTN_X")

(abs "ABS_Y" (xbox360:button "X"))

;; How to differ between named args and recursion? -> We don't, use longer syntax

;; vs

(abs (id "ABS_Y") (input (xbox360:button "X")))


;; EOF ;;
