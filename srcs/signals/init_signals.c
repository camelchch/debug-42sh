/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_signals.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */

/*   By: cyfermie <cyfermie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/27 15:23:51 by cfermier          #+#    #+#             */
/*   Updated: 2018/09/20 13:41:39 by cyfermie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <forty_two_sh.h>

/*
	int sigemptyset(sigset_t *set);
	int sigfillset(sigset_t *set);
	int sigadset(sigset_t *set, int sig);
	int sigdelset(sigset_t *set, int sig);
	int sigprocmask(int how, const sigset_t *nset, sigset_t *oset);
	int sigaction(int signum, const struct sigaction *new, struct sigaction *old);

	struct sigaction
	{
		void	(*handler)(int);
		sigset_t	sa_mask;
		int			sa_flag;
	};


*/

/*
**	Initialize some handlers functions for different signals.
*/

void	init_signals(void)
{
	struct sigaction	new;
	unsigned int		i;
	const int			sig_array[26] = 
	{
		SIGHUP, SIGQUIT, SIGILL, SIGTRAP, SIGABRT, SIGEMT, SIGFPE, \
		SIGBUS, SIGSEGV, SIGSYS, SIGPIPE, SIGALRM, SIGTERM, SIGURG, \
		SIGTSTP, SIGCONT, SIGTTIN, SIGTTOU, SIGIO, SIGXCPU, \
		SIGXFSZ, SIGVTALRM, SIGPROF, SIGINFO, SIGUSR1, SIGUSR2
	};

	sigfillset(&(new.sa_mask));
	new.sa_flags = 0;
	new.sa_handler = &(handle_sigint);
	sigaction(SIGINT, &new, NULL);
	new.sa_handler = &(handle_useless_signals);
	i = 0;
	while (i < (sizeof(sig_array) / sizeof(sig_array[0])))
	{
		sigaction(sig_array[i], &new, NULL);
		++i;
	}
}
