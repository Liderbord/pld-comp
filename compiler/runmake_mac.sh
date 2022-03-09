# to get the required packages (ubuntu 21.10):
#    apt install     antlr4 libantlr4-runtime-dev
# the ANTLRJAR part below is copied from /usr/bin/antlr4
make ANTLRJAR=/usr/share/java/stringtemplate4.jar:/usr/share/java/antlr4.jar:/usr/share/java/antlr4-runtime.jar:/usr/share/java/antlr3-runtime.jar/:/usr/share/java/treelayout.jar ANTLRINC=/Users/emilienmarion/4IF/PLDCompilo/pld-comp/antlr/include ANTLRLIB=/Users/emilienmarion/4IF/PLDCompilo/pld-comp/antlr/lib/libantlr4-runtime.a "$@"

