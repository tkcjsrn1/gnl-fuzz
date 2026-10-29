/* ************************************************************************** */
/*   get_next_line.c  (2025/08/14 version, jkim)                              */
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
	if ((size_t)BUFFER_SIZE > SIZE_MAX - 1)
		return (NULL);
	if (!stash)
		stash = ft_strdup("");
	if (!stash)
		return (NULL);
	stash = read_until_nl(fd, stash);
	if (!stash || *stash == '\0')
	{
		free(stash);
		return (NULL);
	}
	line = extract_line(stash);
	stash = clean_stash(stash);
	return (line);
}

static char	*read_until_nl(int fd, char *stash)
{
	char	*buf;
	ssize_t	rd;

	buf = malloc(BUFFER_SIZE + 1);
	if (!buf)
		return (NULL);
	rd = 1;
	while (!ft_strchr(stash, '\n') && rd > 0)
	{
		rd = read(fd, buf, BUFFER_SIZE);
		if (rd < 0)
		{
			free(buf);
			return (NULL);
		}
		buf[rd] = '\0';
		stash = ft_strjoin(stash, buf);
		if (!stash)
		{
			free(buf);
			return (NULL);
		}
	}
	free(buf);
	return (stash);
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
