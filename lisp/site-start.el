;;; site-start.el --- Site-wide customisations
;;                    that must be loaded before the user's init files

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Up until now (May, 2015), user-init-directory had always been
;; hard-coded to ~/.sxemacs.  Today we take a more "dynamic"
;; approach and support $XDG_CONFIG_HOME as well.  Unfortunately,
;; that flexibility has consequences.  This snippet deals with
;; those consequences. --SY
(when (zerop (length user-init-directory))

  ;; Compute the init directory location
  (setq user-init-directory (find-user-init-directory))

  ;; Make sure the load-path includes any packages the user may have
  ;; in their local setup.
  (unless inhibit-early-packages
    (startup-setup-paths emacs-roots user-init-directory))

  ;; There could possibly now be some more auto-autoloads to load from
  ;; the user's local packages.
  (unless inhibit-autoloads
    (unless inhibit-early-packages
      (packages-load-package-auto-autoloads early-package-load-path)))

  ;; emodules ?

  ;; Reset the lisp init.d directory
  (setq lisp-initd-dir
	(file-name-as-directory
	 (paths-construct-path (list user-init-directory
				     lisp-initd-basename))))

  ;; Lastly, warn the user if multiple candidates for user-init-directory
  ;; were found.
  (let* ((legacy (expand-file-name "~/.sxemacs"))
	 (xdg (expand-file-name "sxemacs"
				(getenv "XDG_CONFIG_HOME")))
	 (fback (paths-construct-path
		 (list (user-home-directory) ".config" "sxemacs")))
	 (dirs (remove-duplicates
		(list legacy xdg fback) :test #'string=))
	 (ndirs (count-if #'file-directory-p dirs)))
    (when (> ndirs 1)
      (lwarn 'multi-initd nil
	"Multiple init directories found:
%S

Currently using: %s

See `display-warning-suppressed-classes' to suppress this warning"
	dirs user-init-directory))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Site-specific additions go here.


;;; End site-start.el
