#ifndef SERVER_HPP
# define SERVER_HPP

# include <cerrno>
# include <csignal>
# include <cstdlib>
# include <cstring>
# include <cctype>
# include <iostream>
# include <map>
# include <set>
# include <stdexcept>
# include <string>
# include <vector>
#include <algorithm>
#include <sstream>


# include <arpa/inet.h>
# include <fcntl.h>
# include <netdb.h>
# include <netinet/in.h>
# include <poll.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <unistd.h>

# define RED "\e[1;31m"
# define WHI "\e[0;37m"
# define GRE "\e[1;32m"
# define YEL "\e[1;33m"

class Server
{
    private:
        int             port;
        std::string     password;
        static bool     stopSignal;
    
    public:
    int	        parseArgs(char** argv);
    static void	signalHandler(int signalNumber);
    void        run();
    
};

int	        printError(const std::string& errorMessage);



#endif