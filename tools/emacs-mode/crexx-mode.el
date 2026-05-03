;;; crexx-mode.el --- Major mode for Rexx language  -*- lexical-binding: t; -*-

;; Author: rvjansen
;; Version: 0.01
;; Keywords: languages
;; Package-Requires: ((emacs "24.4"))

;;; Commentary:
;;
;; Rexx major mode with:
;; - keyword highlighting
;; - top-level label highlighting
;; - indentation
;;
;; Indentation rules:
;; - ordinary top-level labels flush left
;; - method/factory headers: 2 spaces
;; - method/factory bodies: 4 spaces
;; - nested blocks add further indentation

;;; Code:

(require 'subr-x)
(require 'imenu)
(require 'easymenu)

(defgroup crexx-mode nil
  "Major mode for editing Rexx."
  :group 'languages)

(defgroup rexx-indent nil
  "Indentation settings."
  :group 'crexx-mode)

(defcustom rexx-indent-offset 2
  "Normal block indentation."
  :type 'integer
  :group 'rexx-indent)

(defcustom rexx-member-header-indent 2
  "Indentation for METHOD/FACTORY header lines."
  :type 'integer
  :group 'rexx-indent)

(defcustom rexx-member-body-indent 4
  "Indentation for METHOD/FACTORY body lines."
  :type 'integer
  :group 'rexx-indent)

;; ------------------------------
;; Keywords
;; ------------------------------

(defconst rexx-keywords
  '("address" "arg" "assembler" "break" "call" "class" "do" "drop"
    "echo" "else" "end" "exit"
    "factory" "forever" "if" "import" "interface" "interpret" "iterate" "leave" "method"
    "namespace" "nop" "numeric" "options" "otherwise"
    "parse" "procedure" "push" "pull" "expose"
    "queue" "return" "say" "select" "shell" "signal"
    "then" "to" "trace" "until" "upper" "when" "with" "while" "loop")
  "Rexx keywords.")

(defconst rexx-keywords-regexp
  (concat "\\<" (regexp-opt rexx-keywords t) "\\>")
  "Regexp for Rexx keywords.")

;; ------------------------------
;; Labels / members
;; ------------------------------

(defconst rexx-top-label-token-regexp
  "\\(?:\\*\\|[A-Za-z_][A-Za-z0-9_]*\\)"
  "Regexp matching a top-level Rexx label token.")

;; (defconst rexx-label-regexp
;;   (concat "^\\(" rexx-top-label-token-regexp "\\)[ \t]*:")
;;   "Regexp matching a top-level label.")

(defconst rexx-label-regexp
  (concat "^[ \t]*\\(" rexx-top-label-token-regexp "\\)[ \t]*:")
  "Regexp matching a top-level label.")

(defconst rexx-member-start-regexp
  (concat "^\\(" rexx-top-label-token-regexp
          "\\)[ \t]*:[ \t]*\\_<\\(method\\|factory\\)\\_>")
  "Regexp matching a top-level METHOD or FACTORY header.")

(defconst rexx-number-regexp
  "\\b\\([0-9]+\\(?:\\.[0-9]+\\)?\\)\\b"
  "Regexp matching numeric literals.")

(defconst rexx-class-regexp
  "^[ \t]*\\(\\*\\|[A-Za-z_][A-Za-z0-9_]*\\)[ \t]*:[ \t]*\\_<class\\_>"
  "Regexp matching class definitions.")

(defconst rexx-method-regexp
  "^[ \t]*\\(\\*\\|[A-Za-z_][A-Za-z0-9_]*\\)[ \t]*:[ \t]*\\_<method\\_>"
  "Regexp matching method definitions.")

(defconst rexx-factory-regexp
  "^[ \t]*\\(\\*\\|[A-Za-z_][A-Za-z0-9_]*\\)[ \t]*:[ \t]*\\_<factory\\_>"
  "Regexp matching factory definitions.")

(defconst rexx-interface-regexp
  "^[ \t]*\\(\\*\\|[A-Za-z_][A-Za-z0-9_]*\\)[ \t]*:[ \t]*\\_<interface\\_>"
  "Regexp matching interface definitions.")

(defconst rexx-class-or-interface-regexp
  "^[ \t]*\\(\\*\\|[A-Za-z_][A-Za-z0-9_]*\\)[ \t]*:[ \t]*\\_<\\(class\\|interface\\)\\_>"
  "Regexp matching class or interface definitions.")

(defconst rexx-return-type-regexp
  "=[ \t]*\\(\\.[A-Za-z_][A-Za-z0-9_]*\\)"
  "Regexp matching a CREXX return type.")

(defconst rexx-call-regexp
  "\\_<\\([A-Za-z_][A-Za-z0-9_]*\\)\\_>[ \t]*("
  "Regexp matching a function or method call followed by `(`.")

(defconst rexx-member-start-regexp
  (concat "^[ \t]*\\(" rexx-top-label-token-regexp
          "\\)[ \t]*:[ \t]*\\_<\\(method\\|factory\\)\\_>")
  "Regexp matching a METHOD or FACTORY header, optionally indented.")
;; ------------------------------
;; Syntax
;; ------------------------------

(defvar crexx-mode-syntax-table
  (let ((st (make-syntax-table)))
    ;; Block comments: /* ... */
    (modify-syntax-entry ?/ ". 124b" st)
    (modify-syntax-entry ?* ". 23" st)

    ;; Line comments: # ... end-of-line
    (modify-syntax-entry ?# "< b" st)
    (modify-syntax-entry ?\n "> b" st)

    ;; Strings
    (modify-syntax-entry ?\" "\"" st)
    (modify-syntax-entry ?'  "\"" st)

    ;; Backslash is not an escape in Rexx strings
    (modify-syntax-entry ?\\ "." st)

    ;; Underscore is part of words
    (modify-syntax-entry ?_ "w" st)

    st)
  "Syntax table for `crexx-mode'.")

(defun rexx-syntax-propertize (start end)
  "Apply syntax properties between START and END."
  (funcall
   (syntax-propertize-rules
    ("\\(--\\)\\(?:\\s-\\|$\\)" (1 "< b")))
   start end))

(defvar rexx-imenu-generic-expression
  `(
    ("Classes"   ,rexx-class-regexp   1)
    ("Methods"   ,rexx-method-regexp  1)
    ("Factories" ,rexx-factory-regexp 1)
    ("Interfaces" ,rexx-interface-regexp 1)
    ))
;; ------------------------------
;; Font lock
;; ------------------------------

;; (defconst rexx-defining-label-regexp
;;   "^[ \t]*\\(\\*\\|[A-Za-z_][A-Za-z0-9_]*\\)[ \t]*:[ \t]*\\_<\\(class\\|method\\|factory\\)\\_>"
;;   "Regexp matching a label that defines a class, method, or factory.")

(defconst rexx-defining-label-regexp
  "^[ \t]*\\(\\*\\|[A-Za-z_][A-Za-z0-9_]*\\)[ \t]*:[ \t]*\\_<\\(class\\|method\\|factory\\|interface\\)\\_>"
  "Regexp matching a label that defines a class, method, factory, or interface.")

(defvar rexx-font-lock-keywords
  `(
    ;; Class and interface labels: underline like function/method calls.
    (,rexx-class-or-interface-regexp
     (1 '(:inherit font-lock-function-name-face :underline t) prepend))

    ;; Labels that define class/method/factory/interface.
    (,rexx-defining-label-regexp
     (1 font-lock-function-name-face prepend))

    ;; Function/method calls like foo(...)
    (,rexx-call-regexp
     (1 font-lock-function-name-face))
    
    ;; Keywords
    (,rexx-keywords-regexp . font-lock-keyword-face)

    ;; Flush-left top-level labels
    (,rexx-label-regexp (1 font-lock-function-name-face))

    ;; Numbers
    (,rexx-number-regexp (1 font-lock-constant-face))

    ;; Return types: = .int
    (,rexx-return-type-regexp (1 font-lock-type-face))

     ;; All ARG parameters on an arg line
    (rexx-match-arg-variables (0 font-lock-variable-name-face))

    ;; .types
    ("\\.[A-Za-z_][A-Za-z0-9_]*\\>" . font-lock-type-face)

    ;; address / namespace / import targets
    ("address[ \t]+\\(\\<\\w+\\>\\)" (1 font-lock-variable-name-face))
    ("namespace[ \t]+\\(\\<\\w+\\>\\)" (1 font-lock-variable-name-face))
    ("import[ \t]+\\(\\<\\w+\\>\\)" (1 font-lock-variable-name-face))
    )
  "Font-lock rules for `crexx-mode'.")

;; ------------------------------
;; Abbrevs
;; ------------------------------

(define-abbrev-table 'crexx-mode-abbrev-table ())

;; ------------------------------
;; Helper for easymenu
;; ------------------------------

(defvar crexx-mode-map
  (let ((map (make-sparse-keymap)))
    map)
  "Keymap for `crexx-mode'.")

(easy-menu-define crexx-mode-menu crexx-mode-map
  "Menu for `crexx-mode'."
  '("Crexx"
    ["Index..." imenu t]
    "---"
    ["Indent line" rexx-indent-line t]))

;; ------------------------------
;; Helpers
;; ------------------------------

(defun rexx--line-comment-p ()
  "Return non-nil if current line is a line comment."
  (looking-at "^[ \t]*\\(?:--\\|#\\)"))

(defun rexx--block-comment-start-p ()
  "Return non-nil if current line starts a block comment."
  (looking-at "^[ \t]*/\\*"))

(defun rexx--goto-previous-code-line ()
  "Move point to the previous non-blank, non-comment line.
Return non-nil if such a line exists."
  (let ((found nil))
    (while (and (not found)
                (zerop (forward-line -1)))
      (cond
       ((looking-at "^[ \t]*$"))
       ((rexx--line-comment-p))
       ((rexx--block-comment-start-p))
       (t
        (setq found t))))
    found))

(defun rexx--previous-code-line-indent ()
  "Return indentation of the previous non-blank, non-comment line."
  (save-excursion
    (if (rexx--goto-previous-code-line)
        (current-indentation)
      0)))

(defun rexx--previous-code-line-text ()
  "Return text of the previous non-blank, non-comment line, trimmed."
  (save-excursion
    (if (rexx--goto-previous-code-line)
        (string-trim
         (buffer-substring-no-properties
          (line-beginning-position)
          (line-end-position)))
      "")))

(defun rexx--prev-line-opens-block-p ()
  "Return non-nil if the previous code line opens a block."
  (let ((line (downcase (rexx--previous-code-line-text))))
    (or (string-match-p "\\_<\\(do\\|loop\\|select\\)\\_>" line)
        (string-match-p "\\_<then\\_>\\s-*$" line)
        (string-match-p "^\\s-*else\\_>" line))))

(defun rexx--current-line-closer-or-mid-p ()
  "Return `close' for END, `mid' for WHEN/OTHERWISE, or nil."
  (save-excursion
    (beginning-of-line)
    (let ((case-fold-search t))
      (cond
       ((looking-at "\\s-*\\_<end\\_>") 'close)
       ((looking-at "\\s-*\\_<\\(when\\|otherwise\\)\\_>") 'mid)
       (t nil)))))

(defun rexx--doc-comment-start-line-p ()
  "Return non-nil if current line starts a Javadoc-style block comment."
  (save-excursion
    (beginning-of-line)
    (looking-at "^[ \t]*/\\*\\*")))

(defun rexx--inside-doc-comment-p ()
  "Return non-nil if point is in a Javadoc-style /** */ block comment."
  (let ((comment-start (nth 8 (syntax-ppss))))
    (and comment-start
         (save-excursion
           (goto-char comment-start)
           (looking-at "/\\*\\*")))))

(defun rexx--doc-comment-continuation-line-p ()
  "Return non-nil if current line is a leading `*' line inside a /** */ comment."
  (save-excursion
    (beginning-of-line)
    (and (rexx--inside-doc-comment-p)
         ;; Match only doc-comment continuation lines like "   * text".
         ;; Do not match method bodies or other ordinary code.
         (looking-at "^[ \t]*\\*\\(?:[ \t].*\\)?$"))))

(defun rexx--doc-comment-end-line-p ()
  "Return non-nil if current line is the closing `*/' line of a /** */ comment."
  (save-excursion
    (beginning-of-line)
    (and (rexx--inside-doc-comment-p)
         (looking-at "^[ \t]*\\*/"))))

(defun rexx--doc-comment-base-indent ()
  "Return the indentation of the opening /** line for the current doc comment."
  (save-excursion
    (let ((comment-start (nth 8 (syntax-ppss))))
      (if comment-start
          (progn
            (goto-char comment-start)
            (current-indentation))
        (current-indentation)))))

(defun rexx--class-or-interface-line-p ()
  "Return non-nil if current line is a CLASS or INTERFACE label."
  (save-excursion
    (beginning-of-line)
    (looking-at
     "^[ \t]*\\(\\*\\|[A-Za-z_][A-Za-z0-9_]*\\)[ \t]*:[ \t]*\\_<\\(class\\|interface\\)\\_>")))

(defun rexx--member-start-line-p ()
  "Return non-nil if current line is a METHOD/FACTORY header."
  (save-excursion
    (beginning-of-line)
    (looking-at rexx-member-start-regexp)))

(defun rexx--top-level-label-line-p ()
  "Return non-nil if current line is a top-level label."
  (save-excursion
    (beginning-of-line)
    (looking-at rexx-label-regexp)))

(defun rexx--inside-member-p ()
  "Return non-nil if current line is inside a METHOD/FACTORY body.
Header lines themselves do not count as body lines."
  (and (not (rexx--member-start-line-p))
       (save-excursion
         (let ((found nil)
               (done nil))
           (beginning-of-line)
           (while (and (not done) (not (bobp)))
             (forward-line -1)
             (cond
              ((looking-at rexx-member-start-regexp)
               (setq found t
                     done t))
              ((looking-at rexx-label-regexp)
               (setq found nil
                     done t))))
           found))))

(defun rexx--previous-code-line-is-member-header-p ()
  "Return non-nil if the previous code line is a METHOD or FACTORY header."
  (save-excursion
    (and (rexx--goto-previous-code-line)
         (rexx--member-start-line-p))))

(defun rexx--previous-code-line-is-member-header-p ()
  "Return non-nil if the previous code line is a METHOD or FACTORY header."
  (save-excursion
    (and (rexx--goto-previous-code-line)
         (rexx--member-start-line-p))))

(defun rexx--current-line-is-member-body-p ()
  "Return non-nil if current line is inside a METHOD/FACTORY body."
  (rexx--inside-member-p))

(defun rexx-match-arg-variables (limit)
  "Match ARG variable names up to LIMIT.
Highlights every variable name on an ARG line that appears before =."
  (catch 'found
    (while (re-search-forward
            "\\_<arg\\_>\\|\\([A-Za-z_][A-Za-z0-9_]*\\)[ \t]*="
            limit t)
      (cond
       ;; Saw ARG: continue scanning on this line
       ((save-excursion
          (goto-char (match-beginning 0))
          (looking-at "\\_<arg\\_>")))

       ;; Saw a variable before = ; only accept it on an ARG line
       ((match-beginning 1)
        (save-excursion
          (beginning-of-line)
          (when (looking-at "^[ \t]*arg\\_>")
            (set-match-data (list (match-beginning 1) (match-end 1)))
            (throw 'found t)))))))
  nil)
;; ------------------------------
;; Indentation
;; ------------------------------
(defun rexx-calculate-indentation ()
  "Compute desired indentation for the current line."
  (save-excursion
    (let* ((prev-indent (rexx--previous-code-line-indent))
           (kind (rexx--current-line-closer-or-mid-p))
           (indent 0))

      ;; Base indentation by line type.
      (cond
       ;; Class/interface labels are always flush left.
       ((rexx--class-or-interface-line-p)
        (setq indent 0))

       ;; Member header itself: 2 spaces.
       ((rexx--member-start-line-p)
        (setq indent rexx-member-header-indent))

       ;; First body line after a member header: 4 spaces.
       ((rexx--previous-code-line-is-member-header-p)
        (setq indent rexx-member-body-indent))

       ;; Other lines inside a member body: inherit previous indentation,
       ;; but never let it fall below the body baseline of 4.
       ((rexx--current-line-is-member-body-p)
        (setq indent (max prev-indent rexx-member-body-indent)))

       ;; Top-level label: flush left.
       ((rexx--top-level-label-line-p)
        (setq indent 0))

       ;; Ordinary line outside members: inherit previous indentation.
       (t
        (setq indent prev-indent)))

      ;; Closing and mid-block keywords reduce indentation.
      (pcase kind
        ('close
         (setq indent (max 0 (- indent rexx-indent-offset))))
        ('mid
         (setq indent (max 0 (- indent rexx-indent-offset)))))

      ;; Previous line opens a normal Rexx block.
      (when (rexx--prev-line-opens-block-p)
        (setq indent (+ indent rexx-indent-offset)))

      ;; Keep member bodies at least 4 spaces.
      (when (rexx--current-line-is-member-body-p)
        (setq indent (max indent rexx-member-body-indent)))

      (max 0 indent))))

(defun rexx-indent-line ()
  "Indent current line according to Rexx rules."
  (interactive)
  (let ((col (current-column)))
    (cond
     ;; Keep Javadoc-style block comments stable at any indentation:
     ;;     /**
     ;;      * text
     ;;      */
     ;; The opening line stays where the user put it.  Continuation
     ;; lines align one column after the opening indentation.
     ((rexx--doc-comment-start-line-p)
      (indent-line-to (current-indentation)))

     ((rexx--doc-comment-end-line-p)
      (indent-line-to (1+ (rexx--doc-comment-base-indent))))

     ((rexx--doc-comment-continuation-line-p)
      (indent-line-to (1+ (rexx--doc-comment-base-indent))))

     ;; Absolute override: CLASS and INTERFACE definitions are top-level
     ;; declarations in CREXX and must always start in column 0.
     ((save-excursion
        (beginning-of-line)
        (looking-at rexx-class-or-interface-regexp))
      (indent-line-to 0))

     (t
      (indent-line-to (rexx-calculate-indentation))))
    (when (> col (current-indentation))
      (move-to-column col))))

;; ------------------------------
;; Mode
;; ------------------------------

;;;###autoload
(define-derived-mode crexx-mode prog-mode "Rexx"
  "Rexx major mode."
  :syntax-table crexx-mode-syntax-table

  (setq-local font-lock-defaults '(rexx-font-lock-keywords nil t))
  (setq-local indent-line-function #'rexx-indent-line)
  (setq-local syntax-propertize-function #'rexx-syntax-propertize)
  (setq-local local-abbrev-table crexx-mode-abbrev-table)

  (setq-local comment-use-syntax nil)
  (setq-local comment-style 'indent)
  (setq-local comment-start "-- ")
  (setq-local comment-continue "-- ")
  (setq-local comment-end "")
  (setq-local comment-start-skip "\\(?:--\\|#\\)\\s-*")

  (setq-local imenu-generic-expression rexx-imenu-generic-expression)
  (setq-local imenu-case-fold-search t)
  (easy-menu-add crexx-mode-menu crexx-mode-map))

(provide 'crexx-mode)

;;; crexx-mode.el ends here

