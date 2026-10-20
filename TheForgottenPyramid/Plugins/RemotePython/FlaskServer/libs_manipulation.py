import os
import shutil

too_large_file='embed_interpreter/Lib/site-packages/tensorflow/python/_pywrap_tensorflow_internal.pyd'
pylibs='../../../../PyLibs'
sliced_pyd_dir=f'{pylibs}/sliced_pyd'
too_long_path_files=(
    (f'{pylibs}/external','embed_interpreter/Lib/site-packages/tensorflow/include/external'),
    (f'{pylibs}/compiler','embed_interpreter/Lib/site-packages/tensorflow/compiler'),
    (f'{pylibs}/xla','embed_interpreter/Lib/site-packages/tensorflow/include/tensorflow/compiler/xla'),
    (f'{pylibs}/mlir_hlo','embed_interpreter/Lib/site-packages/tensorflow/include/xla/mlir_hlo')
)

def split_file(input_path, output_dir):
    chunk_size = 90 * 1024 * 1024 # 90MB slices
    base_name = os.path.basename(input_path)
    with open(input_path, 'rb') as file:
        index = 0
        while True:
            chunk = file.read(chunk_size)
            if not chunk:
                break
            part_path = output_dir + f'/{base_name}.part_{index:03d}'
            with open(part_path, 'wb') as part:
                part.write(chunk)
            print(f"Written {part_path}")
            index += 1

def restore_file(parts_dir, output_path):
    with open(output_path, "wb") as output:
        for part_name in sorted(file for file in os.listdir(parts_dir)):
            with open(f'{parts_dir}/{part_name}', "rb") as part_file:
                output.write(part_file.read())
            print(f"Merged {part_name}")
    print(f"Created: {output_path}")


for recovery,destination in too_long_path_files:
    if not os.path.exists(destination):
        if os.path.exists(recovery):
            shutil.move(recovery,destination)
            print(f'Directory {destination} recovered successfully')
        else:
            print(f'Recovery directory {recovery} does not exist')
    else:
        shutil.move(destination,pylibs)
        print(f'Directory {destination} moved successfully')
if os.path.exists(too_large_file):
    if not os.path.exists(sliced_pyd_dir):
        os.mkdir(sliced_pyd_dir)
    split_file(too_large_file,sliced_pyd_dir)
    os.remove(too_large_file)
    print('slicing finished')
elif os.path.exists(sliced_pyd_dir):
    if len(os.listdir(sliced_pyd_dir)) == 11:
        restore_file(sliced_pyd_dir,too_large_file)
        for file in [f'{sliced_pyd_dir}/{name}' for name in  os.listdir(sliced_pyd_dir)]:
            os.remove(file)
        os.rmdir(sliced_pyd_dir)
        print('Recovery finished')
    else:
        print(f'Recovery directory {sliced_pyd_dir} corrupted!\nRestore {too_large_file} manually')
else:
    print(f'File {too_large_file} not found\nRecovery directory {sliced_pyd_dir} not found\nRestore file manually')