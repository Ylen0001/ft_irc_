/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ylenoel <ylenoel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:03:23 by ylenoel           #+#    #+#             */
/*   Updated: 2025/08/12 15:07:13 by ylenoel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

volatile sig_atomic_t g_running = 1;

void signalHandler(int signum)
{
	(void)signum;
	g_running = 0;
}

int main(int argc, char* argv[])
{
	//Pretty cool convention
	//If we don't need the first argument, we can increment the pointer and decrement the counter
	++argv; --argc;

	// 	Now we can check for the real number of arguments
	if (argc != 2) {
		std::cerr << "Usage: " << "./ircserv <port> <password>" << std::endl;
		return EXIT_FAILURE;
	}

	try {
		signal(SIGPIPE, SIG_IGN);
		signal(SIGINT, signalHandler); // Capture CTRL+C
		Server test(atoi(argv[0]), argv[1]);
		test.run();
	} catch(const std::exception& e) {
		std::cerr << "Server error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
		
	return 0;
}