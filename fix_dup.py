import re

with open('engine/render/VulkanBackend.cpp', 'r') as f:
    content = f.read()

# I want to just drop everything from `void VulkanBackend::CopyBufferToImage` line 1004 to line 1159 (CreateTexture and DestroyTexture) 
# and keep the ones that I had from line 985, wait no.
# I had implemented `CopyBufferToImage`, `CreateTexture`, `DestroyTexture` at the very top (lines 14 to 157 in my earlier `view_file` which got moved to the bottom by `fix_vk.py`).
# Wait, `fix_vk.py` moved them to the bottom, but I had already implemented `CopyBufferToImage` in the `replace_file_content` that I did earlier! Wait, I replaced `return true;\n    }\n\n` with `CreateDescriptorSetLayout` and `CopyBufferToImage`.
# Let's just remove the duplicated ones.

# Find the first `CopyBufferToImage`
first_copy = content.find("void VulkanBackend::CopyBufferToImage")
# Find the second `CopyBufferToImage`
second_copy = content.find("void VulkanBackend::CopyBufferToImage", first_copy + 1)

# Find the start of CreateTexture
create_texture = content.find("bool VulkanBackend::CreateTexture", second_copy)

# The second copy of CopyBufferToImage is what we want to remove
new_content = content[:second_copy] + content[create_texture:]

with open('engine/render/VulkanBackend.cpp', 'w') as f:
    f.write(new_content)
