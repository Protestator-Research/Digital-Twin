% CPSthesis.cls
% Autor: Javier Moreno (moreno@cs.uni-kl.de)
% Package to serve as a template for any thesis work developed in Design of 
% Cyber-Physical Systems workgroup of University of Kaiserslautern 

\NeedsTeXFormat{LaTeX2e}
\newcommand{\CPSthesisDate}{2013/10/24} % Version date
\ProvidesClass{CPSthesis}[\CPSthesisDate\space LaTeX Template for different kind of thesis]

\newcommand{\@bachelortitlepages}{}
\newcommand{\@mastertitlepages}{}
\newcommand{\@phdtitlepages}{}
\newcommand{\@praktikumtitlepages}{}
\newcommand{\@maketitlepages}{\@mastertitlepages}  %default: master

% begin document options
\DeclareOption{bachelor}{\renewcommand{\@maketitlepages}{\@bachelortitlepages}}
\DeclareOption{master}{\renewcommand{\@maketitlepages}{\@mastertitlepages}}
\DeclareOption{phd}{\renewcommand{\@maketitlepages}{\@phdtitlepages}}
\DeclareOption{praktikum}{\renewcommand{\@maketitlepages}{\@praktikumtitlepages}}

\DeclareOption*{\ClassWarning{CPSthesis}{Unknown document option "\CurrentOption"}}
\ProcessOptions
% end document options
 
\LoadClass[a4paper,11pt,twoside,openright]{book}
 
\RequirePackage{CPS}	% load associated style file CPS.sty


% Front part setup

\newcommand{\@degreeprogram}{\@MissingArg{Degree program is missing!}}
\newcommand{\degreeprogram}[1]{%
	\renewcommand{\@degreeprogram}{#1}
}

\newcommand{\@place}{\@MissingArg{Place of studies is missing!}}
\newcommand{\place}[1]{%
	\renewcommand{\@place}{#1}
}

\newcommand{\@supervisors}{\@MissingArg{Supervisors are missing!}}

\newcommand{\supervisors}[1]{%
	\renewcommand{\@supervisors}{#1}
}

\newcommand{\@matrnumber}{\@MissingArg{Matriculation Number is missing!}}
\newcommand{\matrnumber}[1]{%
	\renewcommand{\@matrnumber}{#1}
}

%\newcommand{\@gegenstand}{\@MissingArg{Gegenstand??}}
%\newcommand{\gegenstand}[1]{%
%	\renewcommand{\@gegenstand}{#1}
%}

%\newcommand{\@semester}{\@MissingArg{Semester??}}
%\newcommand{\semester}[1]{%
%	\renewcommand{\@semester}{#1}
%}

% Submission Date
\newcounter{@submissionday}
\newcounter{@submissionmonth}
\newcounter{@submissionyear}

\setcounter{@submissionday}{\the\day}
\setcounter{@submissionmonth}{\the\month}
\setcounter{@submissionyear}{\the\year}

\newcommand{\submissiondate}[3]{
	\setcounter{@submissionyear}{#1}
	\setcounter{@submissionmonth}{#2}
	\setcounter{@submissionday}{#3}
}

%% ----------------------------------------------------------

\newcommand{\@company}{\@MissingArg{Company is missing!}}
\newcommand{\company}[1]{%
\renewcommand{\@company}{#1}
}

\newcommand{\@companyurl}{\@MissingArg{Company URL is missing!}}
\newcommand{\companyurl}[1]{%
\renewcommand{\@companyurl}{#1}
}

\newcommand{\@companyphone}{\@MissingArg{Company telephone is missing!}}
\newcommand{\companyphone}[1]{%
\renewcommand{\@companyphone}{#1}
}

\newcommand{\@license}{\@MissingArg{License text is missing!}}
\newcommand{\license}[1]{%
\renewcommand{\@license}{#1}
}


% Titlepage

\newcommand{\@bachelortitle}{%
\begin{titlepage}%
\rule{0mm}{1mm}
\vspace*{10mm}
%\noindent Left side \hfill right side\\
\begin{center}%
  \setlength{\unitlength}{1cm}
  \begin{picture}(12,3)
  %\thinlines\put(0,0){\framebox(12,4){}}
  \put(0,1.8){\parbox{12cm}{\centering\LARGE\bf%
    \setlength{\baselineskip}{20pt}%
    \@title}}
  \end{picture}
    \vskip 10mm%
    {\large\sc\@author}
    \vskip 25mm%
    {\Large B\,A\,C\,H\,E\,L\,O\,R\,\,\,\,T\,H\,E\,S\,I\,S}%
    \vskip 20mm%
    {\large Submitted at the} \vskip 1mm {Faculty of Computer Science}  \vskip 1mm {Design 
    of Cyber-Physical Systems Workgroup} \vskip 4mm {University of Kaiserslautern} \vskip 4mm {as part 
    of the degree program}
    \vskip 4mm%
    {\large\sc\@degreeprogram}%
    \vskip 4mm%
    {\large under supervision of}
    \vskip 4mm%
    {\large \@supervisors}
    \vskip 4mm%
    {\large in \@place}%
    \vskip 10 mm%
    {\large \monthname[\value{@submissionmonth}] \arabic{@submissionyear}}%
%   {\large im \@submissionmonth\ \@submissionyear}%
\end{center}\par
  \vfil\null
\end{titlepage}%
}

\newcommand{\@mastertitle}{%
\begin{titlepage}%
\rule{0mm}{1mm}
\vspace*{10mm}
%\noindent Left side \hfill right side\\
\begin{center}%
  \setlength{\unitlength}{1cm}
  \begin{picture}(12,3)
  %\thinlines\put(0,0){\framebox(12,4){}}
  \put(0,1.8){\parbox{12cm}{\centering\LARGE\bf%
    \setlength{\baselineskip}{20pt}%
    \@title}}
  \end{picture}
    \vskip 10mm%
    {\large\sc\@author}
    \vskip 25mm%
    {\Large M\,A\,S\,T\,E\,R\,\,\,\,T\,H\,E\,S\,I\,S}%
    \vskip 20mm%
    {\large Submitted at the} \vskip 1mm {Faculty of Computer Science}  \vskip 1mm {Design 
    of Cyber-Physical Systems Workgroup} \vskip 4mm {University of Kaiserslautern} \vskip 4mm {as part 
    of the degree program}
    \vskip 4mm%
    {\large\sc\@degreeprogram}%
    \vskip 4mm%
    {\large under supervision of}
    \vskip 4mm%
    {\large \@supervisors}
    \vskip 4mm%
    {\large in \@place}%
    \vskip 10 mm%
    {\large \monthname[\value{@submissionmonth}] \arabic{@submissionyear}}%
%   {\large im \@submissionmonth\ \@submissionyear}%
\end{center}\par
  \vfil\null
\end{titlepage}%
}

\newcommand{\@phdtitle}{%
\begin{titlepage}%
\rule{0mm}{1mm}
\vspace*{10mm}
%\noindent Left side \hfill right side\\
\begin{center}%
  \setlength{\unitlength}{1cm}
  \begin{picture}(12,3)
  %\thinlines\put(0,0){\framebox(12,4){}}
  \put(0,1.8){\parbox{12cm}{\centering\LARGE\bf%
    \setlength{\baselineskip}{20pt}%
    \@title}}
  \end{picture}
    \vskip 10mm%
    {\large\sc\@author}
    \vskip 25mm%
    {\Large P\,h\,D\,\,\,\,T\,H\,E\,S\,I\,S}%
    \vskip 20mm%
    {\large Submitted at the} \vskip 1mm {Faculty of Computer Science}  \vskip 1mm {Design 
    of Cyber-Physical Systems Workgroup} \vskip 4mm {University of Kaiserslautern} \vskip 4mm {as part 
    of the degree program}
    \vskip 4mm%
    {\large\sc\@degreeprogram}%
    \vskip 4mm%
    {\large under supervision of}
    \vskip 4mm%
    {\large \@supervisors}
    \vskip 4mm%
    {\large in \@place}%
    \vskip 10 mm%
    {\large \monthname[\value{@submissionmonth}] \arabic{@submissionyear}}%
%   {\large im \@submissionmonth\ \@submissionyear}%
\end{center}\par
  \vfil\null
\end{titlepage}%
}

\newcommand{\@praktikumtitle}{%
\begin{titlepage}%
\rule{0mm}{1mm}
\vspace*{10mm}
%\noindent Left side \hfill right side\\
\begin{center}%
  \setlength{\unitlength}{1cm}
  \begin{picture}(12,3)
  %\thinlines\put(0,0){\framebox(12,4){}}
  \put(0,1.8){\parbox{12cm}{\centering\LARGE\bf%
    \setlength{\baselineskip}{20pt}%
    \@title}}
  \end{picture}
    \vskip 10mm%
    {\large\sc\@author}
    \vskip 25mm%
    {\Large P\,R\,A\,K\,T\,I\,K\,U\,M}
    \vskip 20mm%
    {\large Submitted at the} \vskip 1mm {Faculty of Computer Science}  \vskip 1mm {Design 
    of Cyber-Physical Systems Workgroup} \vskip 4mm {University of Kaiserslautern} \vskip 4mm {as part 
    of the degree program}
    \vskip 4mm%
    {\large\sc\@degreeprogram}%
    \vskip 4mm%
    {\large under supervision of}
    \vskip 4mm%
    {\large \@supervisors}
    \vskip 4mm%
    {\large in \@place}%
    \vskip 10 mm%
    {\large \monthname[\value{@submissionmonth}] \arabic{@submissionyear}}%
%   {\large im \@submissionmonth\ \@submissionyear}%
\end{center}\par
  \vfil\null
\end{titlepage}%
}

% Copyright

\newcommand{\strictlicense}{%
	{\license{All Rights Reserved}}
}

\newcommand{\cclicense}{%
  {\license{This work is published under the conditions of the 
\textsl{Creative Commons License Attribution--Non\-Commercial--No\-Derivatives} 
(CC BY-NC-ND)---see
\url{http://creativecommons.org/licenses/by-nc-nd/3.0/}.}}}

%\strictlicense
\cclicense	% Creative Commons License (default)

\newcommand{\@makelicense}{ 
  \sbox\@tempboxa{\@license}%
  \ifdim \wd\@tempboxa >\hsize
    \begin{sloppypar}\noindent\@license\end{sloppypar}
  \else
    \global \@minipagefalse
    \hb@xt@\hsize{\hfil\box\@tempboxa\hfil}%
  \fi
} 

\newcommand{\@copyrightpage}{%
\newpage
\setcounter{page}{2}
\thispagestyle{plain}
\rule{0mm}{1mm}
\vspace*{75mm}
\begin{center}
\copyright\ Copyright\ \arabic{@submissionyear}\ \@author
\end{center}
\vspace{8mm}
\@makelicense
%\begin{center}
%\includegraphics[width=2.0cm]{images/by-nc-nd-eu}
%\end{center}
}

% Company Page 
\newcommand{\@companypage}{%
\newpage
\setcounter{page}{2}
\thispagestyle{plain}
\rule{0mm}{1mm}
\begin{center}
\vspace{25mm} 
{Praktikum Position:}
\\
\vspace{3mm}
{\Large\sf\@company \\ \mbox{}}
\\
{\sf \@companyphone}
\\
{\sf \@companyurl}
\\
\vspace{45mm} 
{Contact Person:}
\\
\vspace{3mm} 
{\Large\sf\@supervisor\\ \mbox{}}
\end{center}
}

%Declaration 
\newcommand{\@declarationpage}{%
\chapter*{Declaration}
\noindent
I hereby declare and confirm that this thesis is entirely the result of my own original work. 
Where other sources of information have been used, they have been indicated as such and properly 
acknowledged. I further declare that this or similar work has not been submitted for credit elsewhere.
\par
\vspace{10mm}
\noindent
%\@place, \the\year-\leadingzero{\month}-\leadingzero{\day} %http://de.wikibooks.org/wiki/LaTeX-W%F6rterbuch:_today 
\@place, {\usdate\formatdate{\value{@submissionday}}{\value{@submissionmonth}}{\value{@submissionyear}}}
\par
\vspace{12mm}
\noindent
\@author
}

%------------ Struktur der Frontseiten -------------------

\renewcommand{\@mastertitlepages}{
\@mastertitle
\@declarationpage
}
     
\renewcommand{\@phdtitlepages}{
\@phdtitle
\@declarationpage
}

\renewcommand{\@bachelortitlepages}{
\@bachelortitle
\@declarationpage
}

\renewcommand{\@praktikumtitlepages}{
\@praktikumtitle
\@companypage
\@declarationpage
}

%------------ Automatischer Eintrag der PDF Metadaten ------------

\renewcommand{\maketitle}{	%replace standard LaTeX-command
\hypersetup{
		pdftitle={\@title},
		pdfauthor={\@author},
		pdfsubject={\@degreeprogram, \@place},
		pdfcreator={LaTeX using class CPSthesis},
		pdfproducer={},
		pdfkeywords={}
}%
\@maketitlepages
}

%%-------------------------------------------------------------

\usepackage{CPSheadings}

%%-------------------------------------------------------------

\endinput

