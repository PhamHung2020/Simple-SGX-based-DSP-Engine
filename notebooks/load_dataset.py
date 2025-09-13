
import os
from sklearn.model_selection import train_test_split

NEXMARK_DATA_QUERIES = {
    'Q1 Map': './nexmark_dataset/q1',
    'Q2 Filter': './nexmark_dataset/q2_filter',
    'Q2 Map': './nexmark_dataset/q2_map',
    'Q3 Filter Auction': './nexmark_dataset/q3_filter_auction',
    'Q3 Filter Person': './nexmark_dataset/q3_filter_person',
    'Q3 Join': './nexmark_dataset/q3_join/actual_data',
    'Q3 Map': './nexmark_dataset/q3_map',
    'Q4 Join': './nexmark_dataset/q4_join_auction_bid/actual_data',
    'Q4 Map': './nexmark_dataset/q4_map',
    'Q4 Max': './nexmark_dataset/q4_max',
    'Q4 Avg': './nexmark_dataset/q4_avg',
    'Q5 Count_Auction': './nexmark_dataset/q5_count_by_auction',
    'Q6 Filter': './nexmark_dataset/q6_filter',
    'Q6 Max': './nexmark_dataset/q6_max',
    'Q6 Avg Partition': './nexmark_dataset/avg_partition',
}

NEXMARK_DATA_MAPPING = {
    'Map': {
        'dirs': [
            './nexmark_dataset/q1', # 245
            './nexmark_dataset/q2_map', # 249
            './nexmark_dataset/q3_map', # 384
            './nexmark_dataset/q4_map', # 384
        ],
        'label': 0
    },
    'Filter': {
        'dirs': [
            './nexmark_dataset/q2_filter',
            './nexmark_dataset/q3_filter_auction',
            './nexmark_dataset/q3_filter_person',
            './nexmark_dataset/q6_filter',
        ],
        'label': 1
    },
    'Join': {
        'dirs': [
            './nexmark_dataset/q3_join/actual_data',
            './nexmark_dataset/q4_join_auction_bid/actual_data',
        ],
        'label': 2
    },
    'Max_GroupBy': {
        'dirs': [
            './nexmark_dataset/q6_max',
            './nexmark_dataset/q4_max',
        ],
        'label': 3
    },
    'Avg_Sliding': {
        'dirs': [
            './nexmark_dataset/q4_avg',
        ],
        'label': 4
    },
    'Avg_Partition': {
        'dirs': [
            './nexmark_dataset/avg_partition',
        ],
        'label': 5
    },
    'Count_GroupBy': {
        'dirs': [
            './nexmark_dataset/q5_count_by_auction',
        ],
        'label': 6
    },
}

NEXMARK_CLASS_NAMES = [
    'Map', 
    'Filter', 
    'Join', 
    'Max', 
    'Avg', 
    'Avg (Partition)',
    'Count',
]

SECURE_SGX_CLASS_NAMES = [
    'Map', 
    'Filter', 
    'Reduce',
]

SECURE_SGX_MAPPING = {
    'map': {
        'dirs': [
            './secure_sgx_dataset/map',
        ],
        'label': 0
    },
    'filter': {
        'dirs': [
            './secure_sgx_dataset/filter',
        ],
        'label': 1
    },
    'reduce': {
        'dirs': [
            './secure_sgx_dataset/reduce',
        ],
        'label': 2
    },
}

def load_nexmark_data_queries():
    data_files = {}
    for operator in NEXMARK_DATA_QUERIES:
        dir = NEXMARK_DATA_QUERIES[operator]
        data_files[operator] = {
            'files': [os.path.join(dir, f) for f in os.listdir(dir) if f.endswith('.csv')],
        }
    
    return data_files


def load_nexmark_data(is_split=True, is_random=False, train_size=0.8, val_size=0.1):
    data_files = {}
    for operator in NEXMARK_DATA_MAPPING:
        data_files[operator] = {
            'files': [],
            'label': NEXMARK_DATA_MAPPING[operator]['label']
        }

        for dir in NEXMARK_DATA_MAPPING[operator]['dirs']:
            files = [f for f in os.listdir(dir) if f.endswith('.csv')]
            data_files[operator]['files'].extend([os.path.join(dir, file) for file in files])
    

    return split_nexmark_data(data_files, is_random=is_random, train_size=train_size, val_size=val_size) if is_split else data_files


def split_nexmark_data(data_files, is_random=False, train_size=0.8, val_size=0.1):
    if is_random:
        for operator, data in data_files.items():
            if val_size is not None:
                train_files, test_files = train_test_split(data['files'], train_size=train_size+val_size, random_state=42)
                train_files, val_files = train_test_split(train_files, train_size=train_size/(train_size+val_size), random_state=42)
                data_files[operator]['train_files'] = train_files
                data_files[operator]['val_files'] = val_files
                data_files[operator]['test_files'] = test_files
            else:
                train_files, test_files = train_test_split(data['files'], train_size=train_size, random_state=42)
                data_files[operator]['train_files'] = train_files
                data_files[operator]['test_files'] = test_files

    else:
        for operator in data_files:
            if val_size is not None:
                if operator == 'Join':
                    data_files[operator]['train_files'] = data_files[operator]['files'][:504]
                    data_files[operator]['val_files'] = data_files[operator]['files'][504:756]
                    data_files[operator]['test_files'] = data_files[operator]['files'][756:]
                elif operator == 'Map':
                    data_files[operator]['train_files'] = data_files[operator]['files'][:494]
                    data_files[operator]['val_files'] = data_files[operator]['files'][494:741]
                    data_files[operator]['test_files'] = data_files[operator]['files'][741:]
                elif operator == 'Filter':
                    data_files[operator]['train_files'] = data_files[operator]['files'][:676]
                    data_files[operator]['val_files'] = data_files[operator]['files'][676:976]
                    data_files[operator]['test_files'] = data_files[operator]['files'][976:]
                elif operator == 'Max_GroupBy':
                    data_files[operator]['train_files'] = data_files[operator]['files'][:730]
                    data_files[operator]['val_files'] = data_files[operator]['files'][730:1094]
                    data_files[operator]['test_files'] = data_files[operator]['files'][1094:]
                else:
                    train, test = train_test_split(data_files[operator]['files'], train_size=0.5, shuffle=True, random_state=42)
                    train, val = train_test_split(train, train_size=0.9, shuffle=True, random_state=42)
                    data_files[operator]['train_files'] = train
                    data_files[operator]['val_files'] = val
                    data_files[operator]['test_files'] = test
            else:
                if operator == 'Join':
                    data_files[operator]['train_files'] = data_files[operator]['files'][:504]
                    data_files[operator]['test_files'] = data_files[operator]['files'][504:]
                elif operator == 'Map':
                    data_files[operator]['train_files'] = data_files[operator]['files'][:494]
                    data_files[operator]['test_files'] = data_files[operator]['files'][494:]
                elif operator == 'Filter':
                    data_files[operator]['train_files'] = data_files[operator]['files'][:676]
                    data_files[operator]['test_files'] = data_files[operator]['files'][676:]
                elif operator == 'Max_GroupBy':
                    data_files[operator]['train_files'] = data_files[operator]['files'][:547]
                    data_files[operator]['test_files'] = data_files[operator]['files'][1094:]
                else:
                    train, test = train_test_split(data_files[operator]['files'], train_size=0.5, shuffle=True, random_state=42)
                    data_files[operator]['train_files'] = train
                    data_files[operator]['test_files'] = test
    
    return data_files


def load_secure_sgx_data(is_split=True, is_random=False, train_size=0.8, val_size=0.1):
    data_files = {}
    for operator in SECURE_SGX_MAPPING:
        data_files[operator] = {
            'files': [],
            'label': SECURE_SGX_MAPPING[operator]['label']
        }

        for dir in SECURE_SGX_MAPPING[operator]['dirs']:
            files = [f for f in os.listdir(dir) if f.endswith('.csv')]
            data_files[operator]['files'].extend([os.path.join(dir, file) for file in files])

    return split_secure_sgx_data(data_files, is_random=is_random, train_size=train_size, val_size=val_size) if is_split else data_files


def split_secure_sgx_data(data_files, is_random=False, train_size=0.8, val_size=0.1):
    for operator, data in data_files.items():
        train_files, test_files = train_test_split(data['files'], train_size=train_size, random_state=42)
        data_files[operator]['train_files'] = train_files
        data_files[operator]['test_files'] = test_files
        # data_files[operator] = {
        #     'files': train_files,
        #     'label': data['label']
        # }

        # data_files[operator] = {
        #     'files': test_files,
        #     'label': data['label']
        # }

    return data_files


if __name__ == '__main__':
    data_files = load_nexmark_data()
    for operator, data in data_files.items():
        print(f"Operator: {operator}, Label: {data['label']}, Files: {len(data['files'])}")
        for file in data['files']:
            print(f"  - {file}")