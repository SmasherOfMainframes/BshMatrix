CXX			= gcc

CXXFLAGS 	= -g -Wall

LIBS		= -lncursesw

OUTPUT		= cRain
OBJS		= cRain.c

$(OUTPUT): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBS) -o $(OUTPUT)
