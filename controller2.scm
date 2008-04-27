 (controls
  (toggle-button (id "toggle-button"))
  (join-abs      (id "join-abs")
                 (min -1000)
                 (max  1000)))


(controller-definition
 (controls
  (join-abs (join-axis))
  (toggle   (toggle-button))
             (connect btn0 join-abs:btn0-out))
  (xbox     (xbox360driver))
  (uinput   (uinput (name "Hello World")
                    (btn BTN_A BTN_B BTN_C)
                    (abs ABS_X
                         (ABS_Y (min -32768)
                                (max 32767)))
                    (rel))))
 (connections
  (link xbox:absX1    join-abs:abs0)
  (link xbox:absX2    join-abs:abs0)
  (link join-abs:abs1 uinput:abs0))

 (link xbox:btnX uinput:btn0)
 (link xbox:btnY uinput:btn1)
 (link xbox:btnB uinput:btn2))

;; EOF ;;
