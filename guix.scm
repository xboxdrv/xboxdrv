;; Xbox360 USB Gamepad Userspace Driver
;; Copyright (C) 2019 Ingo Ruhnke <grumbel@gmail.com>
;;
;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

(set! %load-path
  (cons* "/ipfs/QmetP3eCAM9q3VPCj9BvjFdWkTA7voycebnXjyfc2zacFE/guix-cocfree_v0.0.0-45-g0fe3c86"
         %load-path))

(use-modules (guix build-system cmake)
             ((guix licenses) #:prefix license:)
             (guix packages)
             (guix utils)
             (gnu packages boost)
             (gnu packages glib)
             (gnu packages gtk)
             (gnu packages libusb)
             (gnu packages linux)
             (gnu packages pkg-config)
             (gnu packages python)
             (gnu packages python-xyz)
             (gnu packages xorg)
             (guix-cocfree utils))

(define %source-dir (dirname (current-filename)))

(define-public xboxdrv
  (package
   (name "xboxdrv")
   (version (version-from-source %source-dir))
   (source (source-from-source %source-dir))
   (build-system cmake-build-system)
   (arguments
    `(#:tests? #f ; no tests
      #:phases
      (modify-phases
       %standard-phases
       (add-after
        'install 'wrap-program
        (lambda* (#:key outputs inputs #:allow-other-keys)
          (let* ((out (assoc-ref outputs "out"))
                 (python (assoc-ref inputs "python"))
                 (python-version ,(version-major+minor (package-version python)))
                 (pythonpath (cons (string-append out "/lib/python"
                                                  python-version
                                                  "/site-packages")
                                   (search-path-as-string->list
                                    (or (getenv "PYTHONPATH") "")))))
            (wrap-program (string-append out "/bin/xboxdrvctl")
                          `("PYTHONPATH" ":" prefix ,pythonpath))))))
      #:configure-flags
      (list
       ;; Configuration cannot find GTK2 without the two following flags.
       (string-append "-DGTK2_GDKCONFIG_INCLUDE_DIR="
                      (assoc-ref %build-inputs "gtk+-2")
                      "/lib/gtk-2.0/include")
       (string-append "-DGTK2_GLIBCONFIG_INCLUDE_DIR="
                      (assoc-ref %build-inputs "glib")
                      "/lib/glib-2.0/include"))))
   (native-inputs
    `(("pkg-config" ,pkg-config)
      ("glib:bin" ,glib "bin")))
   (inputs
    `(("boost" ,boost)
      ("dbus-glib" ,dbus-glib)
      ("libusb" ,libusb)
      ("libudev" ,eudev)
      ("bluez" ,bluez)
      ("libx11" ,libx11)
      ("glib" ,glib)
      ("gdk-pixbuf" ,gdk-pixbuf)
      ("gtk+-2" ,gtk+-2)
      ("python" ,python)
      ("python-dbus" ,python-dbus)))
   (synopsis "Xbox360 USB Gamepad Userspace Driver")
   (description "Xboxdrv is a Xbox/Xbox360 gamepad driver for Linux
that works in userspace. It is an alternative to the xpad kernel
driver and has support for Xbox1 gamepads, Xbox360 USB gamepads and
Xbox360 wireless gamepads. The Xbox360 guitar and some Xbox1 dancemats
might work too. The Xbox 360 racing wheel is not supported, but
shouldn't be to hard to add if somebody is interested.")
   (home-page "https://gitlab.com/xboxdrv/xboxdrv/")
   (license license:gpl3+)))

xboxdrv

;; EOF ;;
