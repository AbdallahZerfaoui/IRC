/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkeil <tkeil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/24 17:50:46 by tkeil             #+#    #+#             */
/*   Updated: 2025/07/09 20:52:36 by tkeil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Server.hpp"
# include "Channel.hpp"

Channel::Channel(const std::string& name, std::unordered_map<int, Client>& clients) : _name(name), _key(""), _topic(""), _is_private(false), _members(), _operators(), _clients_ref(clients)
{	
}

Channel::Channel(Channel&& other) : _name(std::move(other._name)), _key(std::move(other._key)), _topic(std::move(other._topic)), _is_private(other._is_private), _members(std::move(other._members)), _operators(std::move(other._operators)), _clients_ref(other._clients_ref) {}

void Channel::add_client(int client_fd)
{
	if (_members.find(client_fd) == _members.end())
	{
		_members.insert(client_fd);
		std::cout << "Client FD " << client_fd << " added to channel " << _name << std::endl;
	}
	else
	{
		std::cerr << "Client FD " << client_fd << " is already in channel " << _name << std::endl;
	}
}

size_t Channel::remove_client(int client_fd)
{
	if (_members.find(client_fd) != _members.end())
	{
		if (!_members.erase(client_fd))
			return (0);
		std::cout << "Client FD " << client_fd << " removed from channel " << _name << std::endl;
	}
	else
	{
		std::cerr << "Client FD " << client_fd << " not found in channel " << _name << std::endl;
		return (0);
	}
	return (1);
}

void Channel::add_operator(int client_fd)
{
	if (_members.find(client_fd) == _members.end())
	{
		_members.insert(client_fd);
		std::cout << "Client FD " << client_fd << " added to the operator list of the channel " << _name << std::endl;
	}
	else
	{
		std::cerr << "Client FD " << client_fd << " is already an operator of the channel " << _name << std::endl;
	}
}

size_t Channel::remove_operator(int client_fd)
{
	if (_operators.find(client_fd) != _operators.end())
	{
		if (!_operators.erase(client_fd))
			return (0);
		std::cout << "Client FD " << client_fd << " was removed from the operator list of the channel " << _name << std::endl;
	}
	else
	{
		std::cerr << "Client FD " << client_fd << " is not an operator of the channel " << _name << std::endl;
		return (0);
	}
	return (1);
}

std::set<int> Channel::get_members() const
{
	return _members;
}

// void Server::send_reply(int fd, int code, const std::vector<std::string>& params, const std::string& msg)
// {
//     std::ostringstream out;
//     out << ':' << _hostname << ' ' << code << ' ';
//     for (size_t i = 0; i < params.size(); ++i)
// 	{
//         if (i)
// 			out << ' ';
//         out << params[i];
//     }
//     out << " :" << msg << "\r\n";
//     ssize_t bytes_sent = send(fd, out.str().c_str(), out.str().size(), 0);
// 	if (bytes_sent == -1)
//     {
//         std::cerr << "send() failed for client FD " << fd << ": " << std::strerror(errno) << std::endl;
//         throw std::runtime_error("send() failed");
//     }
//     if (static_cast<size_t>(bytes_sent) < msg.size())
//     {
//         std::cerr << "Warning: Partial send() for client FD " << fd << std::endl;
//         throw std::runtime_error("Partial send(), incomplete message sent");
//     }
// }

void Channel::broadcast_message(const std::string& message, int sender_fd) const
{
	for (const auto& member_fd : _members)
	{
		if (member_fd != sender_fd)
		{
			try
			{
				std::string msg = message;
				_clients_ref.at(member_fd).send(msg);
			}
			catch (const std::exception& e)
			{
			    std::cerr << "Failed to broadcast a message to client: " << e.what() << std::endl;
				return ;
			}
		}
	}
}

bool Channel::has_member(int client_fd) const
{
	return _members.find(client_fd) != _members.end();
}

bool Channel::requires_key() const
{
	return _is_private;
}

void Channel::set_channel_key(const std::string& key)
{
	_key = key;
	if (!_key.empty())
		_is_private = true;
	else
		_is_private = false;
	std::cout << "Channel " << _name << " key set to: " << _key << std::endl;
}

const std::string& Channel::get_channel_key() const { return _key; }

std::string Channel::get_name() const
{
	return _name;
}