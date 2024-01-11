%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Trim Size: 10.25in x 7.5in            
%% Text Area: 8.5in (include Runningheads) x 6in
%% ws-jca.cls   :  23-5-08
%% Tex file to use with ws-jca.tex written in Latex2E.
%% The content, structure, format and layout of this style file is the
%% property of World Scientific Publishing Co. Pte. Ltd.
%% Copyright 1995, 2003 by World Scientific Publishing Co.
%% All rights are reserved.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{ws-jca}[1994/07/13 v1.2u Standard LaTeX document class]
\@twosidetrue\@mparswitchtrue\@twocolumnfalse
\newif\if@openright
\@openrighttrue
\newif\if@restonecol
\newif\if@draft
\newdimen\draftrule
\draftrule=0pt
%
%
\DeclareOption{splissue}{\@splissuetrue}
\DeclareOption{draft}{\@drafttrue\setlength\overfullrule{5pt}\draftrule0.01pt}%
\DeclareOption{final}{\setlength\overfullrule{0pt}}
%
\ExecuteOptions{a4paper,11pt,twoside,onecolumn,final}
\ProcessOptions
%
\hoffset=-1.5cm
\voffset=-0.5cm
%
\newcommand\partname{Part}
\newcommand\appendixname{Appendix}
\newcommand\abstractname{Abstract}
\newcommand\bibname{References}%
\newcommand\contentsname{Contents}
\newcommand\indexname{Index}
\newcommand\figurename{Fig.}
\newcommand\tablename{Table}
%
\newcommand\today{\ifcase\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi
    \space\number\day, \number\year}
\newcount\minute
\newcount\hour
\def\currenttime{%
    \minute\time
    \hour\minute
    \divide\hour60
    \the\hour:\multiply\hour60\advance\minute-\hour\the\minute}
%
\newdimen\trimheight
\newdimen\trimwidth
\newdimen\typeheight
\newdimen\typewidth
\newdimen\tempdimen
\newdimen\tablewidth
\newdimen\normaltextheight
\newbox\tempbox
\newdimen\tablewd
%%
\def\foliofont{\normalfont\fontsize{9}{11}\selectfont}
\def\xfoliofont{\normalfont\fontsize{11}{14}\selectfont}
\def\bibfont{\normalfont\fontsize{10}{12}\selectfont}
\def\xbibfont{\normalfont\fontsize{11}{14}\selectfont}
\let\rhfont\foliofont
\def\catchlinefont{\fontsize{9}{11}\selectfont}
\def\titlefont{\fontsize{11}{14}\bfseries\boldmath\selectfont\centering{}}
\let\authorfont\foliofont
\let\abstractfont\catchlinefont
\def\affiliationfont{\small\itshape}
\def\datefont{\normalfont\small}
\def\extractfont{\footnotesize\itshape{}}
\def\sectionfont{\bfseries\boldmath\rightskip2pc plus1fill{}}
\def\subsectionfont{\bfseries\boldmath\itshape\rightskip2pc plus1fill{}}
\def\subsubsectionfont{\itshape\rightskip2pc plus1fill{}}
\def\paragraphfont{\bfseries\boldmath}
\def\subparagraphfont{\itshape}
\def\figurecaptionfont{\normalfont\small}
\def\tablefont{\normalfont\small}
\def\tablecaptionfont{\normalfont\small}
\def\tabnotefont{\normalfont\small}
%
\def\@xi{11}
\def\@xiiipt{13}
\def\@xiv{14}
\def\@xvipt{16}
\def\@xviiipt{18}
%
\DeclareMathSizes{28}{28}{25}{20}
\DeclareMathSizes{24}{24}{21}{18}
\DeclareMathSizes{22}{22}{19}{17}
\DeclareMathSizes{20}{20}{17}{15}
\DeclareMathSizes{18}{18}{15}{10}
\DeclareMathSizes{12}{11}{8}{5}
\DeclareMathSizes{11}{11}{8}{5}
\DeclareMathSizes{10}{10}{7}{5}
\DeclareMathSizes{9.5}{9}{7}{5}
\DeclareMathSizes{9}{9}{7}{5}
\DeclareMathSizes{8}{8}{6}{5}
\DeclareMathSizes{7}{7}{5}{5}
\DeclareMathSizes{6}{6}{5}{5}
\DeclareMathSizes{5}{5}{5}{5}
%
\renewcommand\normalsize{%
   \@setfontsize\normalsize\@xipt{14} %\@xivpt
   \abovedisplayskip 8\p@\@plus4\p@\@minus2\p@
   \abovedisplayshortskip\abovedisplayskip
   \belowdisplayshortskip\abovedisplayshortskip
   \belowdisplayskip\abovedisplayskip
   \let\@listi\@listI}
\normalsize
%
\newcommand\small{%
   \@setfontsize\small\@ixpt{11}%
   \abovedisplayskip 8\p@ \@plus3\p@ \@minus4\p@
   \abovedisplayshortskip \z@ \@plus2\p@
   \belowdisplayshortskip 4\p@ \@plus2\p@ \@minus2\p@
   \def\@listi{\leftmargin\leftmargini
               \topsep 4\p@ \@plus2\p@ \@minus2\p@
               \parsep 2\p@ \@plus\p@ \@minus\p@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip}
%
\newcommand\footnotesize{%
   \@setfontsize\footnotesize\@ixpt{11}%
   \abovedisplayskip 6\p@ \@plus2\p@ \@minus4\p@
   \abovedisplayshortskip \z@ \@plus\p@
   \belowdisplayshortskip 3\p@ \@plus\p@ \@minus2\p@
   \def\@listi{\leftmargin\leftmargini
               \topsep 3\p@ \@plus\p@ \@minus\p@
               \parsep 2\p@ \@plus\p@ \@minus\p@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip}
%
\newcommand\scriptsize{\@setfontsize\scriptsize\@viipt\@viiipt}
\newcommand\tiny{\@setfontsize\tiny\@vpt\@vipt}
\newcommand\large{\@setfontsize\large\@xiipt{14}}%
\newcommand\Large{\@setfontsize\Large\@xivpt{16}}%%
\newcommand\LARGE{\@setfontsize\LARGE\@xviipt{22}}
\newcommand\huge{\@setfontsize\huge\@xxpt{25}}
\newcommand\Huge{\@setfontsize\Huge\@xxvpt{30}}
\setlength\smallskipamount{3\p@ \@plus 1\p@ \@minus 1\p@}
\setlength\medskipamount{6\p@ \@plus 2\p@ \@minus 2\p@}
\setlength\bigskipamount{12\p@ \@plus 4\p@ \@minus 4\p@}
%
\DeclareOldFontCommand{\rm}{\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\scshape}{\@nomath\sc}
\DeclareRobustCommand\cal{\@fontswitch\relax\mathcal}
\DeclareRobustCommand\mit{\@fontswitch\relax\mathnormal}
\DeclareOldFontCommand{\bi}{\bfseries\itshape}{\bfseries\itshape}
\renewcommand{\rmdefault}{cmr}%
%

\trimheight10.2truein    %measure is 10.25in
\addtolength\trimheight{.047in}
\trimwidth7.50truein 
\typeheight51.11pc       %measure is 8.5in (include running head)
\addtolength\typeheight{.1cm}
%
\setlength\parindent{1.5em}%10\p@
\setlength\headheight{7\p@}%6\P@
\setlength\headsep   {20pt}%15
\setlength\topskip   {8\p@}%{10\p@}
\setlength\footskip{18pt}%18pt
\setlength\maxdepth{4\p@}
\setlength\@maxdepth\maxdepth
\setlength\textwidth{6in}
\setlength\textheight\typeheight
\advance\textheight-\headheight
\advance\textheight-\headsep
%\advance\textheight\topskip%Comment
\advance\textheight-3pt  %comment
\setlength\normaltextheight{\textheight}
\setlength\oddsidemargin{0.75in}%
\setlength\evensidemargin{\trimwidth}
\addtolength\evensidemargin{-\textwidth}
\addtolength\evensidemargin{-\oddsidemargin}
\setlength\marginparwidth{.75in}
\setlength\marginparsep{7\p@}
\setlength\marginparpush{5\p@}
\setlength\topmargin{.65926in}%
\setlength\footnotesep{6\p@}%6pt
\setlength{\skip\footins}{20\p@ \@plus 5\p@ \@minus 2\p@}
\setlength\floatsep    {20\p@ \@plus 2\p@ \@minus 2\p@}
\setlength\textfloatsep{20\p@ \@plus 2\p@ \@minus 4\p@}
\setlength\intextsep   {18\p@ \@plus 2\p@ \@minus 2\p@}
\setlength\dblfloatsep    {18\p@ \@plus 2\p@ \@minus 2\p@}
\setlength\dbltextfloatsep{20\p@ \@plus 2\p@ \@minus 4\p@}
\setlength\@fptop{0\p@}
\setlength\@fpsep{8\p@ \@plus 2fil}
\setlength\@fpbot{0\p@ \@plus 1fil}
\setlength\@dblfptop{0\p@ \@plus 1fil}
\setlength\@dblfpsep{8\p@ \@plus 2fil}
\setlength\@dblfpbot{0\p@ \@plus 1fil}
\setlength\partopsep{0\p@}
%
\setlength\lineskip{1\p@}
\setlength\normallineskip{1\p@}%
\renewcommand\baselinestretch{}%{}
\setlength\parskip{0\p@}
\setlength\parsep{0\p@}
\@lowpenalty   51
\@medpenalty  151
\@highpenalty 301
\@beginparpenalty -\@lowpenalty
\@endparpenalty   -\@lowpenalty
\@itempenalty     -\@lowpenalty
%
%
\newcommand\maketitle{\par
  \begingroup
   \renewcommand\thefootnote{\@fnsymbol\c@footnote}%
    \def\@makefnmark{\rlap{\@textsuperscript{\normalfont\@thefnmark}}}%
    \long\def\@makefntext##1{\noindent
%   \@textsuperscript{\normalfont\@thefnmark}{\lowercase{##1}}}%
    \@textsuperscript{\normalfont\@thefnmark}{{##1}}}%
      \newpage
      \global\@topnum\z@
      \@maketitle
    \thispagestyle{plain}\@thanks
  \endgroup
  \setcounter{footnote}{0}%
  \global\let\thanks\relax
  \global\let\maketitle\relax
  \global\let\@maketitle\relax
  \global\let\@thanks\@empty
  \global\let\@author\@empty
  \global\let\@title\@empty
  \global\let\title\relax
  \global\let\author\relax
}
\def\title#1{\gdef\@title{{#1}}}
\def\subtitle#1{\titlefont :~#1}
\def\@maketitle{%
  \newpage
  \null
  \vspace*{-10pt}
  \vspace*{-29pt}% to have the bylines on the beginning of the page
  \@clinebuf
  \vspace*{52pt}
\vspace*{-.5cm}
  \begin{center}%
  \let\footnote \thanks
  {\titlefont\@title\par}\par%
  \vspace*{22pt}
  \@aabuffer
  \end{center}%
}
%
\def\bm#1{\mathchoice
    {\mbox{\boldmath$#1$}}%
          {\mbox{\boldmath$#1$}}%
          {\mbox{\boldmath$\scriptstyle#1$}}%
          {\mbox{\boldmath$\scriptscriptstyle#1$}}}
%
\def\thebepage{\thepage}
\def\theeepage{\pageref{eof}}

\newenvironment{history}{%
    \addvspace{-2pt plus-2pt minus-2pt}
   \authorfont\begin{center}\vskip-\lastskip
}{%
   \end{center}
    \addvspace{15pt plus2pt minus2pt}
}

\def\email#1{\it #1\par}
\def\http#1{\it http://#1\par}
\def\url#1{\it #1\par}
\def\received#1{Received~#1\par}
\def\revised#1{Revised~#1\par}
\def\accepted#1{Accepted~#1\par}
\def\comby#1{\par\addvspace{1\baselineskip}Communicated\ by~#1\par}
%
%
\newcount\clcount
\def\@clinebuf{}
\def\catchline#1#2#3#4#5{\expandafter\def\expandafter\@clinebuf\expandafter
        {\@clinebuf\catchlinefont
        %\noindent Journal of Computational Acoustics\par
        %\noindent Journal of Computational Acoustics,
        %           Vol.\ {#1}, \No\ {#2} (#3)\ \thebepage--\theeepage\par
        % \noindent \copyright\ IMACS\par
        \noindent\vskip-\baselineskip \hphantom{#4 \hskip2em #5}
    }\relax\par
        }%
\def\No{No.}
%
\def\@aabuffer{}
\def\author#1{\expandafter\def\expandafter\@aabuffer\expandafter
    {\@aabuffer{\authorfont{{#1}}}\relax\par
    \vspace*{3pt}}}
\def\address#1{\expandafter\def\expandafter\@aabuffer\expandafter
    {\@aabuffer\affiliationfont{#1}\relax\par
    \vspace*{16pt}}}
%
\def\keywords#1{\par
    {\authorfont{\leftskip18pt\rightskip\leftskip
    \noindent{\it Keywords}\/:\ #1\par}}\par}
%
\def\category#1#2{\par
    \vspace*{8pt}
    {\authorfont{\leftskip18pt\rightskip\leftskip
    \noindent{\it Categories}\/:\ #1:\ #2\par}}\par}
%
\newenvironment{abstract}{\par%
    \abstractfont
    \noindent\leftskip18pt\rightskip18pt
}{%
  \par\addvspace{6pt plus2pt minus1pt}}
%%%%%%%%%%%%
\def\@listI{\leftmargin\leftmargini
            \listparindent\parindent
            \parsep \z@\labelsep.5em
            \topsep 6.5\p@ \@plus3\p@% \@minus2\p@
            \itemsep0\p@}
%
\let\@listi\@listI
\@listi
%
\def\@listii {\leftmargin\leftmarginii
              \labelwidth\leftmarginii
              \advance\labelwidth-\labelsep
              \topsep    3.5\p@ \@plus2\p@ \@minus\p@
              \parsep    \z@
              \itemsep\z@}
\def\@listiii{\leftmargin\leftmarginiii
              \labelwidth\leftmarginiii
              \advance\labelwidth-\labelsep
              \topsep    3.5\p@ \@plus\p@\@minus\p@
              \parsep    \z@
             % \partopsep \p@ \@plus\z@ \@minus\p@
              \itemsep\z@}%   \topsep}
\def\@listiv {\leftmargin\leftmarginiv
              \labelwidth\leftmarginiv
              \advance\labelwidth-\labelsep}
\def\@listv  {\leftmargin\leftmarginv
              \labelwidth\leftmarginv
              \advance\labelwidth-\labelsep}
\def\@listvi {\leftmargin\leftmarginvi
              \labelwidth\leftmarginvi
              \advance\labelwidth-\labelsep}
%
\setlength\leftmargini{3pc}
\setlength\leftmarginii{2.2em}
\setlength\leftmarginiii{1.87em}
\setlength\leftmarginiv{1.7em}
\setlength\leftmarginv{1em}
\setlength\leftmarginvi{1em}
\setlength\leftmargin{\leftmargini}
\setlength\listparindent{\parindent}
\setlength  \labelsep  {.5em}
\setlength  \labelwidth{\leftmargini}
\addtolength\labelwidth{-\labelsep}
\renewcommand\theenumi{\arabic{enumi}}
\renewcommand\theenumii{\alph{enumii}}
\renewcommand\theenumiii{\roman{enumiii}}
\renewcommand\theenumiv{\Alph{enumiv}}
\newcommand\labelenumi{(\theenumi)}
\newcommand\labelenumii{(\theenumii)}
\newcommand\labelenumiii{\theenumiii.}
\newcommand\labelenumiv{\theenumiv.}
\renewcommand\p@enumii{\theenumi}
\renewcommand\p@enumiii{\theenumi(\theenumii)}
\renewcommand\p@enumiv{\p@enumiii\theenumiii}
\newcommand\labelitemi{$\m@th\bullet$}
\newcommand\labelitemii{\normalfont\bfseries --}
\newcommand\labelitemiii{$\m@th\ast$}
\newcommand\labelitemiv{$\m@th\cdot$}
%
\def\enummax#1{\setbox\tempbox=\hbox{#1\hskip\labelsep}%
  \expandafter\global\csname leftmargin\romannumeral\the\@enumdepth\endcsname\wd\tempbox}
%
\def\enumerate{\@ifnextchar[{\@enumerate}{\@enumerate[\csname label\@enumctr\endcsname]}}
%
\def\@enumerate[#1]{\ifnum \@enumdepth >3 \@toodeep\else
      \advance\@enumdepth \@ne\edef\@enumctr{enum\romannumeral\the\@enumdepth}%
      \enummax{#1}\list
      {\csname label\@enumctr\endcsname}{\usecounter
        {\@enumctr}\def\makelabel##1{\hss\llap{##1}}}\fi}
%
\let\Item\item
\newenvironment{enumeroman}{%
  \def\theenumi{\roman{enumi}}\def\theenumii{\alph{enumii}}%
  \def\labelenumi{(\theenumi)}\def\labelenumii{(\theenumii)}%
        \let\item\Item
  \begin{enumerate}%
}{%
  \end{enumerate}}
%
\newenvironment{alphlist}{%
  \def\theenumi{\alph{enumi}}\def\theenumii{\roman{enumii}}%
  \def\labelenumi{(\theenumi)}\def\labelenumii{(\theenumii)}%
        \let\item\Item
  \begin{enumerate}%
}{%
  \end{enumerate}}
%
\newenvironment{arabiclist}{%
    \def\theenumi{\arabic{enumi}}\def\theenumii{\alph{enumii}}
    \def\labelenumi{(\theenumi)}\def\labelenumii{(\theenumii)}%
    \let\item\Item
    \begin{enumerate}
    }{%
    \end{enumerate}}
%
\newenvironment{romanlist}{%
    \def\theenumi{\roman{enumi}}\def\theenumii{\alph{enumii}}
    \def\labelenumi{(\theenumi)}\def\labelenumii{(\theenumii)}%
    \let\item\Item
    \begin{enumerate}
    }{%
    \end{enumerate}}
%
\newenvironment{itemlist}{%
    \def\labelenumi{\labelitemi}
    \let\item\Item
    \begin{enumerate}
    }{%
    \end{enumerate}}
%
\newenvironment{unnumlist}{%
    \let\item\Item
    \leftmargini2pc
    \ifnum \@enumdepth >3 \@toodeep\else
        \advance\@enumdepth \@ne
        \list{}{\itemindent-2pc\topsep6pt
              \def\makelabel##1{\hss\llap{##1}}}%
   \fi
}{%
  \endlist}

%%%%% ALIGN AFTER BULLET ITEMS AND EM DASH - WROTE BY RAJESH
\def\itemize{%
   \leftmargini1pc
   \leftmarginii1.2em
   \leftmarginiii.87em
 \ifnum \@itemdepth >\thr@@\@toodeep\else
   \advance\@itemdepth\@ne
   \edef\@itemitem{labelitem\romannumeral\the\@itemdepth}%
   \expandafter
   \list
     \csname\@itemitem\endcsname
     {\def\makelabel##1{\hss\llap{##1}}}%
 \fi}
\let\enditemize =\endlist

%
\newenvironment{quote}
               {\list{}{\rightmargin18pt\leftmargin18pt}%
                \item[]}
               {\endlist}
%
\newenvironment{extract}{%
    \let\it\upshape
    \extractfont\par\addvspace{6pt plus2pt minus1pt}%
    \noindent\leftskip36pt
}{%
  \par\addvspace{6pt plus2pt minus1pt}}
%
%
\newcommand\authormark[1]{}
\setcounter{secnumdepth}{3}
\newcounter {part}
\newcounter {section}
\newcounter {subsection}[section]
\newcounter {subsubsection}[subsection]
\newcounter {paragraph}[subsubsection]
\newcounter {subparagraph}[paragraph]
\renewcommand\thepart         {\@Roman\c@part}
\renewcommand\thesection      {\arabic{section}}
\renewcommand\thesubsection   {\thesection.\arabic{subsection}}
\renewcommand\thesubsubsection{\thesubsection .\arabic{subsubsection}}
\renewcommand\theparagraph    {\thesubsubsection.\arabic{paragraph}}
\renewcommand\thesubparagraph {\theparagraph.\arabic{subparagraph}}
%
\newcommand\section{\@startsection{section}{1}{\z@}{-22pt \@plus-3pt minus-6pt}{6pt}{\sectionfont}}
\newcommand\subsection{\@startsection{subsection}{2}{\z@}{-17.5pt \@plus-3pt minus-6pt}{6pt}{\subsectionfont}}
\newcommand\subsubsection{\@startsection{subsubsection}{3}{\z@}{-17.5pt \@plus-3pt minus-6pt}{6pt}{\subsubsectionfont}}
\newcommand\paragraph{\@startsection{paragraph}{4}{\z@}{12pt \@plus -3pt}{-.5em}{\paragraphfont}}
\newcommand\subparagraph{\@startsection{subparagraph}{5}{\parindent}{3.25ex \@plus1ex}{-1em}{\subparagraphfont}}
%
\def\@seccntformat#1{{\upshape\csname the#1\endcsname}.\hskip .5em}
%
\newcommand\part{\cleardoublepage
                 \thispagestyle{empty}%
                 \global\@topnum\z@
                 \@afterindentfalse
                 \secdef\@part\@spart}
%
\def\@part[#1]#2{%
 {\parindent0pt
    {\par\vspace*{-18pt}\PNfont {\def\ {\hskip1pt}P\ A\ R\ T}\ \ \thepart\par}%
    \vskip4pt\vrule height1pt width26pc
    \vskip-4pt
    {\par\PTfont #1\par}}
  \@endpart}
%
\def\@spart#1{%
    {\PTfont #1\par}%
    \@endpart}
%
\def\@endpart{\vfil\newpage
              \if@twoside
                \hbox{}%
                \thispagestyle{empty}%
                \newpage
              \fi}
%
\gdef\@author{}
%
\InputIfFileExists{psfig.sty}{}{}
\InputIfFileExists{epsf.sty}{}{}
\usepackage[figuresright]{rotating}
\usepackage{amssymb}
%
\setcounter{topnumber}{3}
\def\topfraction{.98}
\setcounter{bottomnumber}{3}
\def\bottomfraction{.98}
\setcounter{totalnumber}{6}%4
\def\textfraction{.01}
\def\floatpagefraction{.98100}
\setcounter{dbltopnumber}{2}
\def\dbltopfraction{.981}
\def\dblfloatpagefraction{.99}
%

\newlength\abovecaptionskip
\newlength\belowcaptionskip
\setlength\abovecaptionskip{6\p@}% plus3pt minus3pt}
\setlength\belowcaptionskip{4\p@}%{-1\p@}
%
\def\FigName{figure}%
%
%
\def\figurename{Fig.}
\newcounter{figure}
\renewcommand\thefigure{\@arabic\c@figure}
\def\fps@figure{tbp}
\def\ftype@figure{1}
\def\ext@figure{lof}
\def\fnum@figure{\figurename~\thefigure}
\newenvironment{figure}
               {\@float{figure}}
               {\end@float}
\newenvironment{figure*}
               {\@dblfloat{figure}}
               {\end@dblfloat}
%
\def\ArtDir{\jobname/}
\def\figurebox#1#2#3{%
    \def\arg{#3}%
    \ifx\arg\empty
    {\hfill\vbox{\hsize#2\hrule\hbox to #2{%
        \vrule\hfill\vbox to #1{\hsize#2\vfill}\vrule}\hrule}\hfill}%
    \else
    \hbox to \hsize{\hfill{\epsfbox{\ArtDir#3.eps}}\hfill}%
    \fi}
%
\def\PSFIG#1#2#3{\par
  {\hbox to \hsize{%
    \hfill{\psfig{file=\ArtDir#1.eps,width=#2,angle=#3}}\hfill}\par}}%
%

%
%%%%%%%%%%%%%%%%%%%%%%%%%% Tables %%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\newcounter{table}
\def\tablename{Table}
\renewcommand\thetable{\@arabic\c@table}
\def\fps@table{htbp}
\def\ftype@table{2}
\def\ext@table{lot}
\def\fnum@table{\tablename~\thetable}
\newenvironment{table}
               {\@float{table}}
               {\end@float}
\newenvironment{table*}
               {\@dblfloat{table}}
               {\end@dblfloat}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\def\tableformat#1{\centerline{#1}}
\newdimen\tabledim
\long\def\tbl#1#2{%
%\vskip-4pt
\global\setbox\tempbox\hbox{\tablefont #2}%
 \global\tabledim\wd\tempbox
 \tableformat{\vbox{\hsize\tabledim
                   \caption{#1}
                   \box\tempbox}}%
}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%\long\def\ttbl#1#2#3{%
%   \parindent\z@
%   \noindent
%   \tablecaptionfont
%   \caption{#2}%
%   \tablewd\hsize
%   \advance\tablewd-#1
%   \global\divide\tablewd\tw@
%   \ifdim \wd\captionbox<#1
%   \centerline{\unhbox\captionbox}
%   \else
%   {\leftskip\tablewd% plus1fill{}
%   \rightskip\tablewd% plus1fill{}
%   \unhbox\captionbox\par}
%   \fi
%   \vskip-8pt
%   \centerline{#3}
%}%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\newenvironment{InTextTable}{%
    \par\addvspace{16pt plus6pt minus6pt}%
    \begin{center}
    \tablecaptionfont\noindent\ignorespaces
}{%
    \end{center}
    \par\addvspace{17pt plus6pt minus6pt}}
%
%
%%%%%%%%%%%%%%%%%%%%%%%% Caption %%%%%%%%%%%%%%%%%%%%%%%
%
%\newlength\abovecaptionskip
%\newlength\belowcaptionskip
%\setlength\abovecaptionskip{10\p@}
%%\setlength\belowcaptionskip{0\p@}
%\setlength\belowcaptionskip{4\p@}
\long\def\@makecaption#1#2{%
  \figurecaptionfont
  \vskip\abovecaptionskip
  \sbox\@tempboxa{#1. #2}%
  \ifdim \wd\@tempboxa >\hsize
    #1. #2\par
  \else
    \global \@minipagefalse
    \hbox to \hsize{\hfil\box\@tempboxa\hfil}%
  \fi
  \vskip\belowcaptionskip}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\def\tabnotehead{Note}
\def\tabsourcehead{Source}
\newenvironment{tabnote}{%
   \par\addvspace{2pt}
   \setbox\tempbox\vbox\bgroup\hsize\tabledim\parindent12pt
   \tabnotefont
   \@ifnextchar[{\@tabnote}{\@tabnote[\tabnotehead]}%
}{%
        \par\egroup
   \tableformat{\box\tempbox}}
%
\def\@tabnote[#1]{\vskip6pt\noindent\def\@Tempa{#1}\ifx\@Tempa\@empty\hskip0pt\else{\it#1}:\hskip5pt\ignorespaces\fi}
%
\newenvironment{tabfootnote}{%
   \par\addvspace{2pt}
   \setbox\tempbox\vbox\bgroup\hsize\tabledim\parindent12pt
   \tabnotefont
}{%
        \par\egroup
   \tableformat{\box\tempbox}}
%%
\def\tabmark#1{\noindent\hbox{\normalsize$\m@th^{#1}$}\ignorespaces}
%
\newenvironment{tabsource}{%
   \let\tabnotehead\tabsourcehead
   \begin{tabnote}
}{%
   \end{tabnote}
}
%
\def\Hline{%
  \noalign{\ifnum0=`}\fi\hrule \@height 2\arrayrulewidth \futurelet
   \@tempa\@xhline}
%
\def\x{@{\extracolsep{\fill}}}
\def\toprule{\Hline\\[-7.5pt]}
\def\colrule{\\[-7.5pt]\hline\\[-7.5pt]}
\def\TMcolrule{\\[-9.5pt]\hline\\[-9.5pt]}
\def\botrule{\\[-9pt]\Hline}
\def\crule#1{\\[-10pt]#1\\[-6pt]}
%
%%%%%%%%%####################%%%%%%%%%%%%%%%%%%%%%%
%%%%
\setlength\arraycolsep{1.5\p@}
\setlength\tabcolsep{6\p@}
\setlength\arrayrulewidth{.4\p@}
\setlength\doublerulesep{2\p@}
\setlength\tabbingsep{\labelsep}
\skip\@mpfootins = \skip\footins
\setlength\fboxsep{3\p@}
\setlength\fboxrule{.4\p@}
\renewcommand\theequation{\arabic{equation}}
%
\setlength\columnsep{10\p@}
\setlength\columnseprule{0\p@}
%
\def\bstrut{\vrule width0pt depth5pt}
\def\tstrut{\vrule width0pt height10pt}
\jot=3pt
%
%

%%% 23/5/08 - Comment \[ and equation*
%\renewcommand{\[}{\relax\ifmmode\@badmath\else\bgroup\let\eqnnum\relax\let\refstepeqcnt\relax\@equation\fi}
%\renewcommand{\]}{\relax\ifmmode\end@equation\egroup\@endparenv\@doendpe\else\@badmath\fi}
%

%\@namedef{equation*}{\let\eqnnum\relax\let\refstepeqncnt\relax\@equation}
%\@namedef{endequation*}{\end@equation}
%\@namedef{fleqnarray*}{\arraycolsep0pt\begin{eqnarray*}}
%\@namedef{endfleqnarray*}{\end{eqnarray*}}
%
%\renewenvironment{equation}{\@equation}{\end@equation}
%\newenvironment{flequation}{\mathindent0pt\@equation}{\end@equation}
%%% 23/5/08 - Comment \[ and equation*

%
\def\refstepeqcnt{\refstepcounter{equation}}
\def\eqnnum{\displaywidth\linewidth\hbox{\@eqnnum}}
%
\def\@equation{\let\topsep\abovedisplayskip\@beginparpenalty\predisplaypenalty\@endparpenalty\postdisplaypenalty
     \refstepeqcnt
     \trivlist \item[]\leavevmode
       \hbox to\linewidth\bgroup\hfill $\m@th% $
       \displaystyle}%
\def\end@equation{$\hfill\eqnnum\egroup\endtrivlist}
%
\renewenvironment{eqnarray}{%
   \stepcounter{equation}%
   \def\@currentlabel{\p@equation\theequation}%
   \global\@eqnswtrue
   \m@th
   \global\@eqcnt\z@
   \tabskip\@centering
   \let\\\@eqncr
   $$\everycr{}\halign to\displaywidth\bgroup
       \hskip\@centering$\displaystyle\tabskip\z@skip{##}$\@eqnsel
      &\global\@eqcnt\@ne\hskip \tw@\arraycolsep \hfil${##}$\hfil
      &\global\@eqcnt\tw@ \hskip \tw@\arraycolsep
         $\displaystyle{##}$\hfil\tabskip\@centering
      &\global\@eqcnt\thr@@ \hb@xt@\z@\bgroup\hss##\egroup
         \tabskip\z@skip
      \cr}
    {%
      \@@eqncr
      \egroup
      \global\advance\c@equation\m@ne
   $$\global\@ignoretrue
}
%
\renewcommand\footnoterule{%
  \kern-3\p@
  \hrule \@width176pt%pt.3\columwidth
  \kern2.6\p@}
\newcommand\@makefntext[1]{%
    \parindent 1em%
    \noindent
     \@makefnmark#1}
%
\def\@fnsymbol#1{\ensuremath{\ifcase#1\or *\or \dagger\or \ddagger\or
    \mathchar "278\or \mathchar "27B\or \|\or **\or \dagger\dagger
    \or \ddagger\ddagger \else\@ctrerr\fi}}
%
\def\thefootnote{\alph{footnote}}
\def\@makefnmark{{$^{\rm \@thefnmark}$}}
%
\mark{{}{}}
%
\def\ps@plain{%
     \let\@mkboth\@gobbletwo
     \let\@oddhead\@empty
     \def\@oddfoot{\hfill\mbox{\lower12pt\hbox{\foliofont\thepage}}\hfill}%
     \let\@evenhead\@empty
     \let\@evenfoot\@oddfoot}
%
\def\RightMark{}
%
\def\ps@headings{%
    \let\@oddfoot\@empty
    \let\@evenfoot\@empty
    \def\@evenhead{\rhfont\thepage\hspace*{12pt}{\it \leftmark}\hfill}%
    \def\@oddhead{\hfill\rhfont{\it \rightmark}\hspace*{12pt}\thepage}%
    \let\@mkboth\markboth
    \def\authormark##1{\markboth{##1}{\RightMark}}%
    \def\chaptermark##1{\gdef\RightMark{##1}\markright{##1}}}%
    %\def\sectionmark##1{\gdef\RightMark{##1}\markright{##1}}}%
%
\def\ps@myheadings{%
    \let\@oddfoot\@empty\let\@evenfoot\@empty
    \def\@evenhead{\rlap{\foliofont \thepage}\hfill{\rhfont\leftmark}\hfill}%
    \def\@oddhead{\hfill{\rhfont\rightmark}\hfill{\foliofont\thepage}}%
    \let\@mkboth\@gobbletwo
    \let\chaptermark\@gobble
    \let\sectionmark\@gobble}
%
\def\ps@pblind{%
    \let\@mkboth\@gobbletwo
    \def\@evenhead{}%
    \def\@oddhead{}
    \def\@oddfoot{\hfill\mbox{\lower8pc\hbox{}}\hfill}
   \let\@evenfoot\@oddfoot}
%
%
\def\@citex[#1]#2{\if@filesw\immediate\write\@auxout
    {\string\citation{#2}}\fi   
\def\@citea{}\@cite{\@for\@citeb:=#2\do     
    {\@citea\def\@citea{,}\@ifundefined 
    {b@\@citeb}{{\bf ?}\@warning 
    {Citation `\@citeb' on page \thepage \space undefined}} 
    {\csname b@\@citeb\endcsname}}}{#1}} 
 \newif\if@cghi 
\def\cite{\@cghitrue\@ifnextchar [{\@tempswatrue 
    \@citex}{\@tempswafalse\@citex[]}} 
\def\citelow{\@cghifalse\@ifnextchar [{\@tempswatrue 
    \@citex}{\@tempswafalse\@citex[]}} 
\def\@cite#1#2{{$\null^{#1}$\if@tempswa\typeout 
    {WSPC warning: optional citation argument  
    ignored: `#2'} \fi}} 
\newcommand{\citeup}{\cite} 
% 
\def\@refcitex[#1]#2{\if@filesw\immediate\write\@auxout 
    {\string\citation{#2}}\fi 
\def\@citea{}\@refcite{\@for\@citeb:=#2\do 
    {\@citea\def\@citea{, }\@ifundefined 
    {b@\@citeb}{{\bf ?}\@warning 
    {Citation `\@citeb' on page \thepage \space undefined}} 
    \hbox{\csname b@\@citeb\endcsname}}}{#1}} 
 \def\@refcite#1#2{{#1\if@tempswa\typeout 
        {WSPC warning: optional citation argument 
    ignored: `#2'} \fi}} 
 \def\refcite{\@ifnextchar[{\@tempswatrue 
    \@refcitex}{\@tempswafalse\@refcitex[]}} 
% THEOREM
%
\def\@xthm#1#2{\@begintheorem{#2}{\csname the#1\endcsname}{}\ignorespaces}
\def\@ythm#1#2[#3]{\@begintheorem{}{\csname the#1\endcsname}{#3}\ignorespaces}
%
\let\Definitionfont\upshape%
\let\Theoremfont\itshape% changed for Rajes on 23/09/98 - rsanka
\let\Corollaryfont\itshape
\let\Propositionfont\itshape
\let\Lemmafont\itshape
\let\Conjecturefont\itshape
\let\Examplefont\upshape
\let\Theoremheadfont\bfseries
\let\Corollaryheadfont\bfseries
\let\Propositionheadfont\bfseries
\let\Lemmaheadfont\bfseries
\let\Conjectureheadfont\bfseries
\let\Definitionheadfont\bfseries
\let\Remarkfont\upshape
\def\Exampleheadfont{\bfseries}
\def\Remarkheadfont{\bfseries}
%
\def\@begintheorem#1#2#3{\par\addvspace{8pt plus3pt minus2pt}%
              \noindent{\csname#1headfont\endcsname#1\ \ignorespaces#3 #2.}%
              \csname#1font\endcsname\hskip.5em\ignorespaces}
\def\@endtheorem{\par\addvspace{8pt plus3pt minus2pt}\@endparenv}
%
\newtheorem{conjecture}{Conjecture}[section]
\newtheorem{theorem}{Theorem}[section]
\newtheorem{corollary}{Corollary}[section]
\newtheorem{lemma}{Lemma}[section]
\newtheorem{proposition}{Proposition}[section]
\newtheorem{definition}{Definition}[section]
\newtheorem{example}{Example}[section]
\newtheorem{remark}{Remark}[section]
%
\newif\iflogo
\def\prbox{\hbox to \hsize{\hfill\fboxsep0pt\fbox{\phantom{\vrule width5pt height5pt depth0pt}}}\global\logofalse}
%
\newenvironment{proof}{%
\par\addvspace{12pt plus3pt minus3pt}\global\logotrue%
\noindent{\bf Proof.\hskip.5em}\ignorespaces}{%
    \par\iflogo\vskip-\lastskip
    \vskip-\baselineskip\prbox\par
    \addvspace{12pt plus3pt minus3pt}\fi}
%
\newdimen\bibindent
\bibindent=2em
\def\@biblabel#1{#1.}
\newenvironment{thebibliography}[1]
     {\section*{\bibname}\bibfont%
      \list{\@biblabel{\arabic{enumiv}}}%
           {\settowidth\labelwidth{\@biblabel{#1}}%
            \leftmargin\labelwidth
        \labelsep6pt%.33em
            \advance\leftmargin\labelsep
        \itemsep0pt\parsep0pt
            \usecounter{enumiv}%
            \renewcommand\theenumiv{\arabic{enumiv}}}%
      \sloppy\clubpenalty4000\widowpenalty4000%
      \sfcode`\.=\@m}
     {\def\@noitemerr
       {\@latex@warning{Empty `thebibliography' environment}}%
      \endlist
}
\newcommand\newblock{}
%
%\newenvironment{thebibliography}{%
%   \section*{\bibname}%\markboth{References}{References}
%   \bibfont
%   \let\bibitem\item
%   \list{}{\leftmargin2pc
%   \itemindent-2pc\labelwidth0pt\labelsep0pt
%   \parsep0pt\parskip\z@\itemsep0pt}
%}{%
% \endlist}
%%
\newcounter{appendix}
\newcommand\appendix{\par
    \section*{\appendixname}%
    \refstepcounter{appendix}
    \setcounter{figure}{0}
    \setcounter{section}{0}%
        \setcounter{lemma}{0}
        \setcounter{remark}{0}
        \setcounter{theorem}{0}
    \setcounter{definition}{0}
        \setcounter{corollary}{0}
    \setcounter{equation}{0}
    \@addtoreset{equation}{section}
  \renewcommand\thesection{\Alph{section}}%
  \renewcommand\theequation{\Alph{section}.\arabic{equation}}
  \renewcommand\thefigure{\Alph{figure}.\arabic{figure}}
}%
%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%For contents pages %%%%%%%%%%%%%%%%%%%%%%%%
%
\def\journalname{\uppercase{Journal of Computational Acoustics}}
\def\chead#1#2#3{\par
\thispagestyle{empty}
{\centering{%
{\titlefont\journalname}\par
{{\it Vol.~#1,  No.~#2 (#3)}}\par
\vspace*{34pt}
\noindent CONTENTS\par}}
\vspace*{0.5in}}
\def\leaderfill{\leaders\hbox to 0.35em{\hss.\hss}\hfill\hskip1cm}
\def\centry#1#2#3{\par{\xfoliofont
    {\rightskip3pc plus 1fil
    \noindent{#1\par}}
    \vskip-\lastskip\vskip-\baselineskip
    \hbox to \hsize{\hfill{#3}\par}\par
    \noindent\ignorespaces\hspace*{18pt}{\it #2}}\par
    \addvspace{12pt plus1pt minus1pt}\par
    }
%
\def\ehead#1#2#3{\par
    \addvspace{18pt plus1pt minus1pt}
    {\xfoliofont\rightskip4pc plus1fil
    \noindent{\bf #1{#2}{#3}\par}
    \vspace*{4pt}}\par}
\def\eehead#1#2#3{\par
    \addvspace{18pt plus1pt minus1pt}
    {\xfoliofont\rightskip4pc plus1fil
    \noindent{\bf #1{#2}{#3}\par}
    \vspace*{4pt}}\par}
%
\def\cent#1#2#3{\par\xbibfont\addvspace{16pt plus2pt minus2pt}
    {\rightskip4pc plus1fil
    \noindent#1\par}\par
    \vskip-\lastskip\vskip-\baselineskip
    \hbox to \hsize{\hfill#3}\par
    {\noindent\leftskip15pt{\rightskip4pc plus1fil{\it #2}\par}}\par}
%
%%%%%%%%%%%%%%%%%%%% Trimmarks  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% \def\draftnote{\today\quad\currenttime\quad WSPC/130-JCA\qquad\jobname}%
\def\draftnote{\today\quad\currenttime\quad \qquad\jobname}%
%
\def\trimmarks{\nointerlineskip%
   \vbox to 0pt{\parindent0pt\draftnote}%
    \vskip8pt
    \vbox to 0pt{\parindent0pt%
     \vbox to \trimheight{\offinterlineskip\parindent0pt
     \hbox to \trimwidth{%
    \vrule height2pc width0.1pt\raisebox{2pc}{\vrule width2pc height0.1pt}\hfill
     \raisebox{2pc}{\vrule width2pc height0.1pt}\vrule height2pc width0.1pt}\vfill
     \hbox to \trimwidth{%
    \vrule height2pc width0.1pt\vrule width2pc height0.1pt\hfill
     \vrule height2pc width0.1pt\llap{\vrule width2pc height0.1pt}}}}%
     \vbox to 0pt{\offinterlineskip\parindent0pt\vskip\topmargin%\vskip1.3pt
     \tempdimen\normaltextheight
     \advance\tempdimen27pt
     \moveright\@themargin
     \vbox to \tempdimen{\hrule height\draftrule 
     \vbox to 0pt{\vskip\headheight\vskip\headsep%\vskip3pt
                \vrule height\draftrule width\textwidth}%
      \hbox to \textwidth{%
    \vrule width\draftrule \hfill\vrule height\tempdimen width\draftrule}%
       \hrule height\draftrule}}}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% ss %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\cropmarks{\nointerlineskip%
     \vbox to 0pt{\vskip-8.6pt
    \parindent0pt\hspace*{6pt}\draftnote
     \par\advance\trimheight by 2pc
     \vbox to \trimheight{\offinterlineskip\parindent0pt\vskip-2pc
     \hbox to \trimwidth{\raise2.6pc\hbox{\vrule height2pc width0.3pt}\raisebox{2pc}
   {\hspace*{-2.6pc}\vrule width2pc height0.3pt}\hfill\raisebox{2pc}
   {\vrule width2pc height0.3pt}\hspace*{-2.6pc}\raise2.6pc\hbox{\vrule height2pc width0.3pt}}\vfill
     \hbox to \trimwidth{\lower1.2pc\hbox{\vrule height2pc width0.3pt}\raisebox{1.4pc}
        {\hspace*{-2.6pc}\vrule width2pc height0.3pt}\hfill 
    \lower1.2pc\hbox{\vrule height2pc width0.3pt}\rlap{\hspace*{0.6pc}\raisebox{1.4pc}{\vrule width2pc height0.3pt}}}}}
     \vbox to 0pt{\offinterlineskip\parindent0pt\vskip\topmargin%\vskip1.3pt
     \tempdimen\normaltextheight
     \advance\tempdimen24pt
     \moveright\@themargin
     \vbox to \tempdimen{\hrule height\draftrule 
     \vbox to 0pt{\vskip\headheight\vskip\headsep%\vskip3pt
                \vrule height\draftrule width\textwidth}%
      \hbox to \textwidth{\vrule width\draftrule \hfill\vrule height\tempdimen width\draftrule}%
           \hrule height\draftrule}}}%%
\def\xcropmarks{\nointerlineskip%
     \vbox to 0pt{\vskip-8.6pt
    \parindent0pt\hspace*{6pt}\draftnote
     \par\advance\trimheight by 2pc
     \vbox to \trimheight{\offinterlineskip\parindent0pt\vskip-2pc
     \hbox to \trimwidth{\raise2.6pc\hbox{\vrule height2pc width0.3pt}\raisebox{2pc}
   {\hspace*{-2.6pc}\vrule width2pc height0.3pt}\hfill\raisebox{2pc}
   {\vrule width2pc height0.3pt}\hspace*{-2.6pc}\raise2.6pc\hbox{\vrule height2pc width0.3pt}}\vfill
     \hbox to \trimwidth{\lower1.2pc\hbox{\vrule height2pc width0.3pt}\raisebox{1.4pc}
        {\hspace*{-2.6pc}\vrule width2pc height0.3pt}\hfill 
    \lower1.2pc\hbox{\vrule height2pc width0.3pt}\rlap{\hspace*{0.6pc}\raisebox{1.4pc}{\vrule width2pc height0.3pt}}}}}
     \vbox to 0pt{\offinterlineskip\parindent0pt\vskip\topmargin%\vskip1.3pt
     \tempdimen\normaltextheight
     \advance\tempdimen27pt
     \moveright\@themargin
     \vbox to \tempdimen{\hrule height\draftrule 
     \vbox to 0pt{\vskip\headheight\vskip\headsep%\vskip3pt
                \vrule height\draftrule width\textwidth}%
      \hbox to \textwidth{\vrule width\draftrule \hfill\vrule height\tempdimen width\draftrule}%
           \hrule height\draftrule}}}%%
%
\let\trimmarks\cropmarks
%
\def\@outputpage{%
\begingroup           % the \endgroup is put in by \aftergroup
  \let \protect \noexpand
  \@resetactivechars
  \let\-\@dischyph
  \let\'\@acci\let\`\@accii\let\=\@acciii
  \let\\\@normalcr
  \let\par\@@par          %% 15 Sep 87
  \shipout \vbox{%
    \set@typeset@protect
    \aftergroup \endgroup
    \aftergroup \set@typeset@protect
                                % correct? or just restore by ending
                                % the group?
  \if@specialpage
    \global\@specialpagefalse\@nameuse{ps@\@specialstyle}%
  \fi
  \if@twoside
    \ifodd\count\z@ \let\@thehead\@oddhead \let\@thefoot\@oddfoot
         \let\@themargin\oddsidemargin
    \else \let\@thehead\@evenhead
       \let\@thefoot\@evenfoot \let\@themargin\evensidemargin
    \fi
  \fi
  \reset@font
  \normalsize
  \baselineskip\z@skip \lineskip\z@skip \lineskiplimit\z@
    \@begindvi\xcropmarks
    \vskip \topmargin
    \moveright\@themargin \vbox {%
      \setbox\@tempboxa \vbox to\headheight{%
        \vfil
        \color@hbox
          \normalcolor
          \hb@xt@\textwidth {%
            \let \label \@gobble
            \let \index \@gobble
            \let \glossary \@gobble %% 21 Jun 91
            \@thehead
            }%
        \color@endbox
        }%                        %% 22 Feb 87
      \dp\@tempboxa \z@
      \box\@tempboxa
      \vskip \headsep
      \box\@outputbox
      \baselineskip \footskip
      \color@hbox
        \normalcolor
        \hb@xt@\textwidth{%
          \let \label \@gobble
          \let \index \@gobble      %% 22 Feb 87
          \let \glossary \@gobble   %% 21 Jun 91
          \@thefoot
          }%
      \color@endbox
      }%
    }%
\global \@colht \textheight
\stepcounter{page}%
\let\firstmark\botmark
}
%
%
%%%%%%%%%%%%%%%%%%%%%%%% Table of Contents %%%%%%%%%%%%%%%%%%%%%%%
%
\newcommand\@pnumwidth{1.55em}
\newcommand\@tocrmarg{2.55em}
\newcommand\@dotsep{4.5}
\setcounter{tocdepth}{3}
\newcommand\tableofcontents{%
    \section*{\contentsname
        \@mkboth{%
           \contentsname}{\contentsname}}%
    \@starttoc{toc}%
    }
\newcommand\l@section[2]{%
  \ifnum \c@tocdepth >\z@
    \addpenalty\@secpenalty
    \addvspace{1.0em \@plus\p@}%
    \setlength\@tempdima{1.5em}%
    \begingroup
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      \leavevmode \bfseries
      \advance\leftskip\@tempdima
      \hskip -\leftskip
      #1\nobreak\hfil \nobreak\hbox to \@pnumwidth{\hss #2}\par
    \endgroup
  \fi}
\newcommand\l@subsection{\@dottedtocline{2}{1.5em}{2.3em}}
\newcommand\l@subsubsection{\@dottedtocline{3}{3.8em}{3.2em}}
\newcommand\l@paragraph{\@dottedtocline{4}{7.0em}{4.1em}}
\newcommand\l@subparagraph{\@dottedtocline{5}{10em}{5em}}
\newcommand\listoffigures{%
    \section*{\listfigurename
      \@mkboth{\listfigurename}%
              {\listfigurename}}%
    \@starttoc{lof}%
    }
\newcommand\l@figure{\@dottedtocline{1}{1.5em}{2.3em}}
\newcommand\listoftables{%
    \section*{\listtablename
      \@mkboth{%
          \listtablename}{\listtablename}}%
    \@starttoc{lot}%
    }
\let\l@table\l@figure
%
%%%%%%%%%%%%%%%%%%%%%%% Local definitions%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\uc#1{\uppercase{#1}} %elsie 14/5/98 (\u c --- text accent)
\def\mtb{\fboxsep=0pt\fbox{\phantom{e}}}
\def\breakhere{\par\vskip-\lastskip
        \pagebreak}
\def\forcebreak{\vadjust{\pagebreak}}
\def\and{\hbox{\lowercase{and }}}
\def\aand{\\[10pt]{\upshape{and}}\\[-3pt]}

%
\def\enddocument{%
   \label{eof}
   \@enddocumenthook
   \@checkend{document}%
   \clearpage
   \begingroup
     \if@filesw
       \immediate\closeout\@mainaux
       \let\@setckpt\@gobbletwo
       \let\@newl@bel\@testdef
       \@tempswafalse
       \makeatletter \input\jobname.aux
     \fi
     \@dofilelist
     \ifdim \font@submax >\fontsubfuzz\relax
       \@font@warning{Size substitutions with differences\MessageBreak
                  up to \font@submax\space have occured.\@gobbletwo}%
     \fi
     \@defaultsubs
     \@refundefined
     \if@filesw
       \ifx \@multiplelabels \relax
         \if@tempswa
           \@latex@warning@no@line{Label(s) may have changed.
               Rerun to get cross-references right}%
         \fi
       \else
         \@multiplelabels
       \fi
     \fi
   \endgroup
   \deadcycles\z@\@@end}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Process Options %%%%%%%%%%%%%%%%%%%%%%%%
%
\pagestyle{headings}
\pagenumbering{arabic} 
\onecolumn
\frenchspacing
\flushbottom
\def\sloppy{\tolerance=100000\hfuzz=\maxdimen \vfuzz=\maxdimen}
\sloppy
\vbadness=12000
\hbadness=12000
\usepackage[figuresright]{rotating}
\usepackage{amsfonts}
\usepackage{amssymb}
\usepackage{amsmath} 
\endinput

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
