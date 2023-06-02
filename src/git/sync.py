import os
import re

cwd = os.getcwd()
print(f'Current directory: \n\tPath: {cwd}')

dirs = []
files = os.scandir(cwd)
for f in files:
	if os.path.isdir(f):
		dirs.append(os.path.basename(f))
print(f'Found git project folders: \n\tNumber: {len(dirs)}')

remote_repositories = []
with open(os.path.join(cwd, '.remote_repositories')) as file:
	for l in file:
		urls = re.findall('http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\(\), ]|(?:%[0-9a-fA-F][0-9a-fA-F]))+', l)
		if len(urls) > 0:
			url = urls[0]
			for v in ['\n', '\t', ' ', url]:
				l = l.replace(v, '')
			remote_repositories.append({
				'name': os.path.basename(url.replace('.git', '')),
				'link': url,
				'args': l
			})
print(f'Found remote repositories: \n\tNumber: {len(remote_repositories)}')

added_repo = []
updated_repo = []
for repo in remote_repositories:
	if repo['name'] not in dirs:
		added_repo.append(repo)
	else:
		updated_repo.append(repo)
repo_names = [repo['name'] for repo in remote_repositories]
removed_repo = []
for dir in dirs:
	if dir not in repo_names:
		removed_repo.append(dir)
print(f'Sync statics: \n\tAdded: {len(added_repo)} \n\tUpdated: {len(updated_repo)} \n\tRemoved: {len(removed_repo)}')

if len(added_repo) > 0:
	print('Adding repositories: ')
	for r in added_repo:
		print(f'\t{r["link"]}')
		cmd = f'git clone {r["link"]} --depth 1 {r["args"]}'
		os.system(cmd)

if len(updated_repo) > 0:
	print('Updating repositories: ')
	for r in updated_repo:
		print(f'\t{r["link"]}')
		cmd = f'cd \"{os.path.join(cwd, r["name"])}\" && git pull'
		os.system(cmd)

if len(removed_repo) > 0:
	print('Removing repositories: ')
	for r in removed_repo:
		print(f'\t{os.path.join(cwd, r)}')
		cmd = f'rm -fr r \"{os.path.join(cwd, r)}\"'
		os.system(cmd)
	
print('Git repositories synchronization finished.')
		