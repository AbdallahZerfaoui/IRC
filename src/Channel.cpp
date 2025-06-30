/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkeil <tkeil@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/24 17:50:46 by tkeil             #+#    #+#             */
/*   Updated: 2025/06/30 20:33:45 by tkeil            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Channel.hpp"

Channel::Channel(const std::string& name, std::unordered_map<int, Client>& clients) : _name(name), _clients_ref(clients)
{	
}

Channel::Channel(Channel&& other) : _name(std::move(other._name)), _clients(std::move(other._clients)), _clients_ref(other._clients_ref) {}

void Channel::add_client(int client_fd)
{
	if (_clients.find(client_fd) == _clients.end())
	{
		_clients.insert(client_fd);
		std::cout << "Client FD " << client_fd << " added to channel " << _name << std::endl;
	}
	else
	{
		std::cerr << "Client FD " << client_fd << " is already in channel " << _name << std::endl;
	}
}

size_t Channel::remove_client(int client_fd)
{
	if (_clients.find(client_fd) != _clients.end())
	{
		if (!_clients.erase(client_fd))
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

std::set<int> Channel::get_clients() const
{
	return _clients;
}

void Channel::broadcast_message(const std::string& message, int sender_fd) const
{
	for (const auto& member_fd : _clients)
	{
		if (member_fd != sender_fd)
		{
			try
			{
			    _clients_ref.at(member_fd).send("#" + _name + " :" + message);
			}
			catch (const std::exception& e)
			{
			    std::cerr << "Failed to broadcast a message to client: " << e.what() << std::endl;
				return ;
			}
		}
	}
}
