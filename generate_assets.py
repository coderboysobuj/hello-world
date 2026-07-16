import math

def write_obj(filename, vertices, indices, uvs):
    with open(filename, 'w') as f:
        for v in vertices:
            f.write(f"v {v[0]} {v[1]} {v[2]}\n")
        for vt in uvs:
            f.write(f"vt {vt[0]} {vt[1]}\n")
        for vn in vertices: # Just use position as normal for simplicity
            length = math.sqrt(vn[0]**2 + vn[1]**2 + vn[2]**2)
            if length == 0: length = 1
            f.write(f"vn {vn[0]/length} {vn[1]/length} {vn[2]/length}\n")
        for i in range(0, len(indices), 3):
            # 1-based indexing for OBJ
            i1, i2, i3 = indices[i]+1, indices[i+1]+1, indices[i+2]+1
            f.write(f"f {i1}/{i1}/{i1} {i2}/{i2}/{i2} {i3}/{i3}/{i3}\n")

# Generate simple character (A box for body, box for head)
char_verts = [
    # Body (0.8x1.4x0.4)
    [-0.4, 0, -0.2], [0.4, 0, -0.2], [0.4, 1.4, -0.2], [-0.4, 1.4, -0.2],
    [-0.4, 0, 0.2], [0.4, 0, 0.2], [0.4, 1.4, 0.2], [-0.4, 1.4, 0.2],
    # Head (0.5x0.5x0.5) centered on top
    [-0.25, 1.4, -0.25], [0.25, 1.4, -0.25], [0.25, 1.9, -0.25], [-0.25, 1.9, -0.25],
    [-0.25, 1.4, 0.25], [0.25, 1.4, 0.25], [0.25, 1.9, 0.25], [-0.25, 1.9, 0.25]
]
char_uvs = [[0,0], [1,0], [1,1], [0,1]] * 4
char_indices = [
    # Body
    0,1,2, 0,2,3, 1,5,6, 1,6,2, 5,4,7, 5,7,6, 4,0,3, 4,3,7, 3,2,6, 3,6,7, 4,5,1, 4,1,0,
    # Head
    8,9,10, 8,10,11, 9,13,14, 9,14,10, 13,12,15, 13,15,14, 12,8,11, 12,11,15, 11,10,14, 11,14,15, 12,13,9, 12,9,8
]
write_obj('assets/character.obj', char_verts, char_indices, char_uvs)

# Generate simple level (Plane + Ramp + Box)
level_verts = [
    # Floor (50x50)
    [-25, 0, -25], [25, 0, -25], [25, 0, 25], [-25, 0, 25],
    # Ramp (starts at z=5, goes up to z=15, height 3, width 4)
    [-2, 0, 5], [2, 0, 5], [2, 3, 15], [-2, 3, 15],
    [-2, 0, 15], [2, 0, 15], # under ramp support
    # Box obstacle (10x10)
    [10, 0, 10], [15, 0, 10], [15, 2, 10], [10, 2, 10],
    [10, 0, 15], [15, 0, 15], [15, 2, 15], [10, 2, 15]
]
level_uvs = [[v[0]/5.0, v[2]/5.0] for v in level_verts] # tile texture every 5 units
level_indices = [
    # Floor
    0,1,2, 0,2,3,
    # Ramp Top
    4,5,6, 4,6,7,
    # Box obstacle
    10,11,12, 10,12,13, 11,15,16, 11,16,12, 15,14,17, 15,17,16, 14,10,13, 14,13,17, 13,12,16, 13,16,17
]
write_obj('assets/level.obj', level_verts, level_indices, level_uvs)

print("Generated character.obj and level.obj")
