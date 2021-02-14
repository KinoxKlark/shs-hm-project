#include <fstream>

std::vector<std::string> importNames(char *filename)
{
	std::ifstream file;
	std::vector<std::string> names;
	
	file.open(filename, std::ios::in);
	assert(("Identities file can't be open!", file.is_open()));

	std::string line;
	while(std::getline(file, line))
	{
		names.push_back(line);
	}

	file.close();

	return names;
}

std::vector<std::string> importFirstNames(bool men)
{
	if(men) return importNames("data/firstnames_men.txt");
	else return importNames("data/firstnames_women.txt");
}

std::vector<std::string> importLastNames()
{
	return importNames("data/lastnames.txt");
}

std::vector<GaugeInfo> importGauges(char *filename)
{
	std::ifstream file;
	std::vector<GaugeInfo> gauges;
	
	file.open(filename, std::ios::in);
	assert(("Identities file can't be open!", file.is_open()));

	std::string line;
	while(std::getline(file, line))
	{
		gauges.push_back({ (u32)gauges.size(), line });
	}

	file.close();

	return gauges;
}

UserIdentity createUserIdentity(GameData *data)
{
	std::vector<GaugeInfo> *personalities = &data->personalities;
	std::vector<GaugeInfo> *interests = &data->interests;
	
	UserIdentity identity = {};
	identity.personalities.resize(personalities->size());
	for(u32 idx = 0; idx < personalities->size(); ++idx)
	{
		identity.personalities[idx].id = (*personalities)[idx].id;
		identity.personalities[idx].amount = get_random_number_between(0.f, 1.f);
	}

	u8 interest_count = get_random_number_between(3, 5);
	for(u8 idx = 0; idx < interest_count; ++idx)
	{
		u32 interest_id = get_random_number_between(0, (u32)interests->size()-1);
		GaugeInfo *interest = &((*interests)[interest_id]);

		for(u8 idx_interests = 0; idx_interests < identity.interests.size(); ++idx_interests)
		{
			if(identity.interests[idx_interests].id == interest->id)
			{
				identity.interests[idx_interests].id = interest->id;
				identity.interests[idx_interests].amount =
					get_random_number_between(identity.interests[idx_interests].amount, 1.f);

				if(identity.interests[idx_interests].amount > 1.f)
					identity.interests[idx_interests].amount = 1.f;
				goto next_interest;
			}
		}

		identity.interests.push_back({interest->id, get_random_number_between(.3f, 1.f)});

	next_interest:
		interest_id; // Dummy line
	}

	return identity;
	
}
