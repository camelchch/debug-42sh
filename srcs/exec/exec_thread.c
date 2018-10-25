/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_thread.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyfermie <cyfermie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/06/25 11:16:01 by sbrucker          #+#    #+#             */
/*   Updated: 2018/10/21 16:22:58 by cyfermie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <forty_two_sh.h>

/*
 ** Handle pipes and redirections then
 ** launch specified program in child process
 **
 ** $cmd wizard casts explanations:
 ** if cmd[0] is equal to EXEC_THREAD_NOT_BUILTIN,
 ** we know the the passed command
 ** is not a builtin and should be executed by execve,
 ** with path stored in cmd[1] and args in cmd[2],
 **
 ** otherwise if cmd[0] is equal to EXEC_THREAD_BUILTIN,
 ** we know that the command we
 ** have to execute is a builtin, and so should not be executed with execve() but
 ** with a function pointer to the builtin instead,
 ** the function pointer will then be stored in cmd[1] (w/ args in cmd[2]),
 ** and be casted to be executed by this function
 ** (which allows pipes and redirs handling for builtins
 ** without duplicating code)
 */
#include <stdio.h>
static void	close_child_pipe_fds(t_ast *node, t_ast *last_pipe)
{
	t_ast	*ptr;

	ptr = node;
	while (ptr->parent != last_pipe)
		ptr = ptr->parent;
	if (ptr == last_pipe->right)
	{
		printf("close fd=%d  ",*(&(last_pipe->data[1][0])));
		close(*(&(last_pipe->data[1][0])));
		if (last_pipe->parent && last_pipe->parent->type_details == TK_PIPE)
		{
			printf("close fd=%d  ",*(&(last_pipe->parent->data[1][sizeof(int)])));
			close(*(&(last_pipe->parent->data[1][sizeof(int)])));
			}
	}
	else if (ptr == last_pipe->left)
	{
		printf("close fd=%d " ,*(&(last_pipe->data[1][sizeof(int)])));
		close(*(&(last_pipe->data[1][sizeof(int)])));
		}
printf("\n");
}

static void	child_process(void **cmd, t_environ *env, t_exec *exe, \
		t_ast *node,int i)
{
	int		backup_stdout;
	int		pipe_stdout_fd;

	backup_stdout = dup(STDOUT_FILENO);
printf("i=%d\n", i);
	if (!i)
	close(5);
	else
	close(6);
	printf("backup_stdout=%d\n", backup_stdout);
	pipe_stdout_fd = handle_pipes(node);
	handle_redirs(node);
	close(backup_stdout - 1);
	if (cmd)
	{
		log_debug("Exec child process cmd: %p - cmd[0] : %d", cmd, (intptr_t)cmd[0]);
		if ((intptr_t)*cmd == EXEC_THREAD_BUILTIN)
			(*(void (**)(char **, t_environ *, t_exec *))(cmd[1]))\
				(cmd[2], env, exe);
		else
		{
			log_debug(" -> child process path : cmd[1] : %s", cmd[1]);
			if (execve(cmd[1], cmd[2], env->environ) == -1)
			{
				log_error("Execve() not working");
			}
		}
	}
	if (pipe_stdout_fd)
	{
		close(pipe_stdout_fd);
		handle_redir_fd(STDOUT_FILENO, backup_stdout);
	}
	if (!cmd || (intptr_t)*cmd != EXEC_THREAD_BUILTIN)
		exit(EXIT_FAILURE);
}

/*
 ** Close unessecary pipe inputs
 */

/*
 ** Parent process function when forking
 ** Wait child process to end
 */

static int	parent_process(pid_t child_pid, t_ast *node, \
		t_ast *last_pipe_node)
{
	int		status;
	//	int		waited_pid;
	(void)child_pid;
	(void)node;
	(void)last_pipe_node;

	if (node && last_pipe_node)
		close_child_pipe_fds(node, last_pipe_node);
	status = -2;
	errno = 0;
	status = 0;
	/*
	   waited_pid = waitpid(child_pid, &status, 0);
	   if (waited_pid == -1)
	   {
	   if (errno != EINTR)
	   {
	   log_error("Wait returned -1");
	   ft_putstr_fd("21sh: err: Could not wait child process\n", 2);
	   return (status);
	   }
	   return (130);
	   }
	   if (waited_pid != -1 && waited_pid != child_pid)
	   ft_putstr_fd("21sh: err: Wait terminated for wrong process\n", 2);
	   */
	return (status);
}

/*
 ** Fork here if passed command
 ** is not the exit() builtin function (verified by is_cmd_exit())
 ** pointer stored in cmd[1], then
 ** call the parent_process() function to wait the child process,
 ** and the child_process() function to handle pipes, redirs, and process/builtin
 ** execution.
 ** Forked builtins should be added to this list
 ** More explanation of $cmd in commentary of the child_process() function
 */

static int	should_fork(void **cmd) // devrait s'appeler should_not_fork() lol
{
	void	(*ptr)(char **, t_environ *, t_exec *);

	ptr = *((void (**)(char **, t_environ *, t_exec *))(cmd[1]));
	if ((intptr_t)*cmd == EXEC_THREAD_NOT_BUILTIN || \
			(ptr == &builtin_echo || \
			 ptr == &builtin_return || \
			 ptr == &builtin_test))
		return (1);
	return (0);
}

void		save_all_children(int child_pid)
{
	int		i;

	i = 0;
	while (g_all_children[i])
		i++;
	g_all_children[i] = child_pid;
}

t_exec		*exec_thread(void **cmd, t_environ *env_struct, t_exec *exe, \
		t_ast *node)
{
	pid_t	child_pid;
	t_ast	*last_pipe_node;
	int		*pipe_fds;
	static int		i = 0;

	(void)env_struct;
	if ((last_pipe_node = get_last_pipe_node(node)) && \
			!last_pipe_node->data[1])
	{
		ft_putstr("111111111\n");
		pipe_fds = init_pipe_data(&(last_pipe_node->data), last_pipe_node);
	}
	if (should_fork(cmd))
	{
		ft_putstr("222222222222\n");
		child_pid = fork();
		if (child_pid == -1)
			log_error("Fork() not working");
		else if (child_pid == 0)
		{
			child_process(cmd, env_struct, exe, node, i);
		}
		else
		{
			save_all_children(child_pid);
			g_cmd_status.cmd_running = true;
			g_cmd_status.cmd_pid = child_pid;
			log_trace("Forked process pid: %d", child_pid);
			exe->ret = parent_process(child_pid, node, last_pipe_node);
			//dprintf(1, "-> %d\n", exe->ret);
		}
	}
	else
	{
		ft_putstr("0000000\n");
		child_process(cmd, env_struct, exe, node, i);
	}
	i++;
	return (exe);
}
