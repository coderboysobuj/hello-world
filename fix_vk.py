import re

with open('engine/render/VulkanBackend.cpp', 'r') as f:
    content = f.read()

# We inserted `void VulkanBackend::CopyBufferToImage` up to `DestroyTexture` before `namespace mmo::render {`
# Let's find `namespace mmo::render {`
idx = content.find("namespace mmo::render {")

# Then we find the second `namespace mmo::render {` which I also see in the diff:
# "namespace mmo::render {" at the very end of the diff block.
# Actually, I can just use a regex to extract the methods.
methods = content[content.find("void VulkanBackend::CopyBufferToImage"):idx]

# Remove the methods from the top part
new_content = content[:content.find("// ...")] + "\nnamespace mmo::render {\n" + content[idx + len("namespace mmo::render {"):]

# Now append methods to the end of the namespace block
# Find the last closing brace in the file
last_brace = new_content.rfind("}")

new_content = new_content[:last_brace] + methods + "\n}\n"

with open('engine/render/VulkanBackend.cpp', 'w') as f:
    f.write(new_content)
