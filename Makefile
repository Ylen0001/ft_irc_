NAME = ircserv
CPP = c++
RM = rm -rf
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -g3
SRC = main.cpp Server.cpp Client.cpp Commands.cpp Channel.cpp
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))
SRC_DIR = src
OBJ_DIR = obj
INCLUDES_DIR = include

DEF_COLOR		:= \033[0;39m
RESET			:= \033[0m
ROSE_RED 		:= \033[0;38;2;170;68;101m
CRIMSON_RED		:= \033[38;2;136;13;30m
QUARTZ 			:= \033[0;38;2;154;140;152m
DARK_BLUE 		:= \033[0;38;2;74;78;105m
MIDNIGHT_BLUE 	:= \033[0;38;2;34;34;59m
TEAL 			:= \033[0;38;2;0;169;165m
DUSTY_RED       := \033[38;2;148;93;94m   # #945D5E
COLOMBIA_BLUE   := \033[0;38;2;201;240;255m

all : ascii_art

all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(QUARTZ)Compilation en cours...$(RESET)"
	@$(CPP) $(CPPFLAGS) $(OBJ) -o $(NAME)
	@echo "$(COLOMBIA_BLUE)Exécutable '$(NAME)' crée!$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "$(QUARTZ)Compilation de $< vers $@...$(RESET)"
	@$(CPP) $(CPPFLAGS) -I$(INCLUDES_DIR) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@echo "$(DARK_BLUE)Suppression des .o en cours...$(RESET)"
	@$(RM) $(OBJ) 
	@echo "$(QUARTZ)Suppression effectuée avec succès!$(RESET)"

fclean: clean
	@echo "$(DARK_BLUE)Suppression de l'exécutable en cours...$(RESET)"
	@$(RM) $(NAME) $(OBJ_DIR)
	@echo "$(QUARTZ)Suppression effectuée avec succès!$(RESET)"

re: fclean all

.PHONY: all clean fclean re all

ascii_art:
	@echo "$(ROSE_RED)"
	@cat logo.txt
	@echo "$(RESET)"
	@echo "\n"
