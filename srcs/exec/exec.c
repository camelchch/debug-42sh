/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyfermie <cyfermie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/06/23 13:03:53 by sbrucker          #+#    #+#             */
/*   Updated: 2018/10/21 16:17:49 by cyfermie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <forty_two_sh.h>

/*
** Malloc for the struct t_exec*
** In fail, exit the program with MALLOC_ERROR
*/

t_exec	*create_exec(t_environ *env)
{
	t_exec	*exe;

	exe = (t_exec *)ft_xmemalloc(sizeof(t_exec));
	exe->ret = 0;
	exe->ready_for_exec = 0;
	exe->env = env;
	return (exe);
}

/*
** Main function for executing a local file like './21_sh' or '/bin/ls'
** Look for the presence of the file and for execution rigths.
** Then send everything to exec_thread().
*/

void			exec_local(char **argv, t_environ *env_struct, t_exec *exe, t_ast *node)
{
	char			*cmd;
	struct stat		s_stat;

	cmd = argv[0];
	if (access(cmd, F_OK) != 0)
	{
		ft_putstr_fd(SH_NAME ": no such file or directory: ", 2);
		ft_putendl_fd(cmd, 2);
	}
	else if (access(cmd, X_OK) != 0)
	{
		ft_putstr_fd(SH_NAME ": permission denied: ", 2);
		ft_putendl_fd(cmd, 2);
	}
	else
	{
		if (stat(cmd, &s_stat) == -1)
			return ;
		if (S_ISDIR(s_stat.st_mode))
		{
			ft_putstr_fd(SH_NAME ": ", 2);
			ft_putstr_fd(cmd, 2);
			ft_putstr_fd(": is a directory\n", 2);
		}
		else	
			exec_thread((void *[3]){EXEC_THREAD_NOT_BUILTIN, cmd, argv}, \
			env_struct, exe, node);
	}
}

/*
** Main function for executing a builtin.
** execute builtin function stored in $builtin_ptr with exec_thread
*/

int				exec_builtin(char **argv, t_environ *env_struct, t_exec *exe, \
					t_ast *node)
{
	void	(*builtin_fun_ptr)(char **, t_environ *, t_exec *);

	builtin_fun_ptr = NULL;
	if (!(is_builtin(argv[0], &builtin_fun_ptr)))
		return (0);
	exec_thread(\
		(void *[3]){(void *)EXEC_THREAD_BUILTIN, &builtin_fun_ptr, argv}, \
		env_struct, exe, node);
	return (1);
}

/*
** Main function for executing a binary like 'ls' or 'pwd' or 'sh'
** Look in the PATH environment var and look for the correponding path.
** Then send everything to exec_thread().
*/

void			exec_binary(char **argv, t_environ *env_struct, t_exec *exe, t_ast *node)
{
	char		*prog_path;
	char		*path_entry;
	struct stat	s_stat;

	path_entry = NULL;
	if (env_struct->get_var(env_struct, "PATH"))
		path_entry = env_struct->last_used_elem->val_begin_ptr;
	exe->ret = -2;
	prog_path = isin_path(path_entry, argv[0]);								//{ le_debug("prog path = |%s|\n", prog_path) }
	if (prog_path)
	{
		if (stat(prog_path, &s_stat) == -1)
			return ;
		if (S_ISDIR(s_stat.st_mode))
		{
			ft_putstr_fd(SH_NAME ": ", 2);
			ft_putstr_fd(prog_path, 2);
			ft_putstr_fd(": is a directory\n", 2);
		}
		else
			exec_thread((void *[3]){EXEC_THREAD_NOT_BUILTIN, prog_path, argv}, \
			env_struct, exe, node);
	}
	else
	{
		ft_putstr_fd(SH_NAME": ", 2);
		ft_putstr_fd(argv[0], 2);
		ft_putendl_fd(": command not found", 2);
	}
	ft_strdel(&prog_path);
}

/*
** Main execution function. Assume that *ast exist, is completely correct,
** and can be just executed.
** char **envp comes directly from the main()
*/

t_exec				*exec_cmd(t_ast *root, t_exec *exe)
{
	exe = ast_explore(root, exe);
	int i = -1;
	while (g_all_children[++i])
		waitpid(g_all_children[i], NULL, 0);
	/*if (VERBOSE_MODE)
		ast_debug(root);*/
	if (!exe)
		return (NULL);
	return (exe);
}
