/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/23 22:45:00 by jkim              #+#    #+#             */
/*   Updated: 2025/09/04 16:47:16 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

static char	*read_until_nl(int fd, char *stash);
static char	*extract_line(char *stash);
static char	*clean_stash(char *stash);

char	*get_next_line(int fd)
{
	static char	*stash;
	char		*line;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	if (!stash)
		stash = ft_strdup("");
	if (!stash)
		return (NULL);
	stash = read_until_nl(fd, stash);
	if (!stash || *stash == '\0')
	{
		if (stash)
			free(stash);
		stash = NULL;
		return (NULL);
	}
	line = extract_line(stash);
	if (!line)
		return (free(stash), stash = NULL, NULL);
	stash = clean_stash(stash);
	return (line);
}

static char	*read_until_nl(int fd, char *stash)
{
	char	*buf;
	char	*temp;
	ssize_t	rd;

	buf = malloc(BUFFER_SIZE + 1);
if (!buf)
		return (free(stash), NULL);
	if (!stash)
		return (free(buf), NULL);
	while (!ft_strchr(stash, '\n'))
	{
		rd = read(fd, buf, BUFFER_SIZE);
		if (rd == -1)
			return (free(buf), free(stash), NULL);
		if (rd == 0)
			break ;
		buf[rd] = '\0';
		temp = ft_strjoin(stash, buf);
		if (!temp)
			return (free(buf), free(stash), NULL);
		free(stash);
		stash = temp;
	}
	return (free(buf), stash);
}

static char	*extract_line(char *stash)
{
	size_t	len;
	size_t	i;
	char	*line;

	if (!stash || *stash == '\0')
		return (NULL);
	len = 0;
	while (stash[len] != '\0' && stash[len] != '\n')
		len++;
	if (stash[len] == '\n')
		len++;
	line = malloc(len + 1);
	if (!line)
		return (NULL);
	i = 0;
	while (i < len)
	{
		line[i] = stash[i];
		i++;
	}
	line[i] = '\0';
	return (line);
}

static char	*clean_stash(char *stash)
{
	size_t	i;
	char	*new;

	if (!stash)
		return (NULL);
	i = 0;
	while (stash[i] && stash[i] != '\n')
		i++;
	if (stash[i] == '\n')
		i++;
	if (stash[i] == '\0')
	{
		free(stash);
		return (NULL);
	}
	new = ft_substr(stash, i, ft_strlen(stash) - i);
	free(stash);
	return (new);
}
