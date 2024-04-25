#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cmath>
#include "maskTable.h"

int validIP(std::string ipaddr);
std::string ipFromVector(std::vector<int>& v);
void fillSizes(int count, char* args[], std::vector<int>& v);
void fillOctets(std::string ip, std::vector<int>& v);
void calculate(std::string ip, std::string netmask, std::vector<int> netSizes);
void prettyPrint(std::string s, const int& width);
void addToIP(std::vector<int>& ip, long hostCount);
void duplicateVec(std::vector<int>& source, std::vector<int>& destination);

int main(int argc, char* argv[])
{
	/* Process args */
	if (argc < 4)
	{
		std::cerr << "Not enough arguments." << std::endl << "Usage: [Network IP] [Network Mask] [LAN 1 host count] [LAN 2 host count] [LAN 3 host count]..." << std::endl;
		return 1;
	}
	else
	{
		std::string s = argv[2];
		if (s[0] != '/')
		{
			std::cerr << "Invalid network mask." << std::endl << "Please enter mask in slash notation (eg. /24)" << std::endl;
			return 1;
		}
	}

	/* Check if is valid network address */
	int returnCode = validIP(argv[1]);
	if (returnCode) //If return code is anything other than 0
	{
		std::cerr << "Invalid IP address." << std::endl;
		switch (returnCode)
		{
		case 1:
			std::cerr << "Only numbers permitted in IP address." << std::endl;
			break;
		case 2:
			std::cerr << "4 octets expected." << std::endl;
			break;
		case 3:
			std::cerr << "Octets must be in range [0-255]" << std::endl;
			break;
		default:
			std::cerr << "Unexpected error encountered. What the fuck did you do???" << std::endl;
			break;
		}
		return 1;
	}

	/* Fill vector of subnet sizes */
	std::vector<int> sizes;
	fillSizes(argc, argv, sizes);

	/* Calculate subnets */
	calculate(argv[1], argv[2], sizes);

	return 0;
}

void calculate(std::string ip, std::string netmask, std::vector<int> netSizes)
{
	using namespace std;

	/* Sum all netSizes */
	long sum = 0;
	for (int n : netSizes)
		sum += n;
	sum += 2 * netSizes.size(); //network and broadcast addresses for each LAN

	/* Check if netmask is large enough for required IPs */
	if (hostCount(netmask) < sum)
	{
		cout << "Subnetting Failed." << endl;
		cout << hostCount(netmask) << " (" << str_hostCount(netmask) << ") hosts allocated by " << netmask << endl;
		cout << "Network IPs:\t" << netSizes.size() << endl;
		cout << "Broadcast IPs:\t" << netSizes.size() << endl;
		cout << "Host IPs:\t" << sum - (2 * netSizes.size()) << endl;
		cout << "Total:\t" << sum << " IPs needed." << endl;
	}
	else
	{
		const int printWidth = 17; //length of 255.255.255.255 + 2 space characters, max width item to be printed
		prettyPrint("Subnet", printWidth);
		prettyPrint("Needed Size", printWidth);
		prettyPrint("Allocated Size", printWidth);
		prettyPrint("Network", printWidth);
		prettyPrint("Broadcast", printWidth);
		prettyPrint("Subnet Mask", printWidth);
		cout << endl;

		vector<int> ipOctets;
		fillOctets(ip, ipOctets); //fill ipOctets vector with default ip address
		for (int netIndex = 0; netIndex < netSizes.size(); netIndex++)
		{
			/* Calculate network and broadcast IPs */
			long allocatedSize = hostCount(getNetmask(netSizes[netIndex] + 2));

			string netIP = "NETWORK";
			string broadIP = "BROADCAST";

			vector<int> tempIP(4, -1); //temporary vector has 4 elements initialized to -1 by default
			netIP = ipFromVector(ipOctets);
			duplicateVec(ipOctets, tempIP); //copy contents of ipOctets to temporary vector
			addToIP(ipOctets, allocatedSize - 1); //broadcast IP
			broadIP = ipFromVector(ipOctets);
			duplicateVec(tempIP, ipOctets); //Copy ipOctets back from temporary vector
			addToIP(ipOctets, allocatedSize); //Next subnet network IP

			/* Print everything out to table */
			prettyPrint(to_string(netIndex + 1), printWidth);
			prettyPrint(to_string(netSizes[netIndex]), printWidth);
			prettyPrint(to_string(allocatedSize), printWidth);
			prettyPrint(netIP, printWidth);
			prettyPrint(broadIP, printWidth);
			prettyPrint(getNetmask(netSizes[netIndex] + 2), printWidth);
			cout << endl;
		}
	}
}

void duplicateVec(std::vector<int>& source, std::vector<int>& destination)
{
	for (int i = 0; i < source.size(); i++)
		destination[i] = source[i];
}

void addToIP(std::vector<int>& ip, long hostCount)
{
	const int divisor = 256;
	ip[3] += hostCount;
	ip[2] += (ip[3] / divisor);
	ip[3] %= divisor;
	ip[1] += (ip[2] / divisor);
	ip[2] %= divisor;
	ip[0] += (ip[1] / divisor);
	ip[1] %= divisor;
}

std::string ipFromVector(std::vector<int>& v)
{
	std::string s = "";
	for (int n = 0; n < v.size(); n++)
	{
		s += std::to_string(v[n]);
		if (n != v.size() - 1)
			s += ".";
	}
	return s;
}

void fillOctets(std::string ip, std::vector<int>& v)
{
	/* Fill vector v with each octet as an integer */
	for (int i = ip.length() - 1; i >= 0; i--)
	{
		if (ip[i] == '.')
		{
			v.insert(v.begin(), std::stoi(ip.substr(i + 1, ip.length())));
			ip = ip.substr(0, i);
		}
	}
	v.insert(v.begin(), std::stoi(ip)); //Add first octet to vector to fix fencepost error
}

void prettyPrint(std::string s, const int& width)
{
	using namespace std;
	cout << left << setw(width) << setfill(' ') << s;
}

void fillSizes(int count, char* args[], std::vector<int>& v)
{
	for (int i = 3; i < count; i++) //start i at 3 to ignore program name ip address and subnet mask
	{
		v.push_back(std::stoi(args[i]));
	}
}

int validIP(std::string ipaddr)
{
	std::vector<int> octets;

	/*Check that everything is numbers or dots */
	for (int i = 0; i < ipaddr.length(); i++)
		if (!(ipaddr[i] >= '0' && ipaddr[i] <= '9') && ipaddr[i] != '.')
			return 1;

	/*Load octets into vector*/
	fillOctets(ipaddr, octets);

	/* Check that there are only four octets */
	if (octets.size() != 4)
		return 2;

	/* Make sure all octets are in range [0-255] */
	for (int n : octets)
		if (!(n >= 0 && n < 256))
			return 3;

	return 0; //clean exit from function
}