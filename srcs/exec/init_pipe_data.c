/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_pipe_data.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyfermie <cyfermie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/23 18:19:20 by jjaniec           #+#    #+#             */
/*   Updated: 2018/10/19 19:34:20 by cyfermie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <forty_two_sh.h>

/*
** Create a pipe and store file descriptors in ast node data,
** if pipe_node pointer is specified and parent is also a pipe token,
** recurse in parent
*/
#include <stdio.h>
int		*init_pipe_data(char ***node_data, t_ast *pipe_node_ptr)
{
	int		*pipe_fds;
	int		*pipe_save;
	char	*old_pipe_data_0;

	pipe_fds = ft_xmalloc(sizeof(int) * 2);
	pipe_save = ft_xmalloc(sizeof(int) * 2);
	old_pipe_data_0 = (*node_data)[0];
	free((*node_data)[1]);
	free(*node_data);
	(*node_data) = ft_xmalloc(sizeof(char *) * 3);
	(*node_data)[0] = old_pipe_data_0;
	(*node_data)[1] = (char *)(pipe_fds);
	(*node_data)[2] = NULL;
	pipe(pipe_fds);
	pipe_save[0] = pipe_fds[0];
	pipe_save[1] = pipe_fds[1];
	printf("fd[0]=%d  fd[1]=%d\n", pipe_fds[0], pipe_fds[1]);
	//fcntl(pipe_fds[0], F_SETFL, fcntl(pipe_fds[0], F_GETFL) | O_NONBLOCK);
	log_info("Created pipe w/ fds: %d %d", \
		pipe_fds[0], pipe_fds[1]);
	if (pipe_node_ptr && pipe_node_ptr->parent && \
		pipe_node_ptr->parent->type_details == TK_PIPE)
		init_pipe_data(&(pipe_node_ptr->parent->data), pipe_node_ptr->parent);
		return (pipe_save);
}
