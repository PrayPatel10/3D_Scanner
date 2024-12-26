import os
import cv2
import torch
import numpy as np
from flask import Flask, request, render_template, send_from_directory
from torchvision.transforms import Compose, Resize, ToTensor, Normalize
from PIL import Image
from rembg import remove
import trimesh
from scipy.spatial import Delaunay
import open3d as o3d 
import time 
# Load MiDaS model
model_type = "DPT_Large"  # Choose "DPT_Large", "DPT_Hybrid", or "MiDaS_small"
model = torch.hub.load("intel-isl/MiDaS", model_type)
model.eval()

# Set up model transforms
transform = Compose([
    Resize((384, 384)),
    ToTensor(),
    Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
])

app = Flask(__name__)

UPLOAD_FOLDER = 'uploads/'
DEPTH_FOLDER = 'depth_maps/'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
os.makedirs(DEPTH_FOLDER, exist_ok=True)

@app.route('/')
def index():
    return render_template('webpage.html')

@app.route('/upload', methods=['POST'])
def upload_file():
    if 'files' not in request.files:
        return 'No file part'
    files = request.files.getlist('files')
    
    if not files:
        return 'No selected files'

    depth_filenames = []
    for file in files:
        if file.filename == '':
            continue  # Skip empty filenames

        filepath = os.path.join(UPLOAD_FOLDER, file.filename)
        file.save(filepath)

        # Remove background and generate depth map
        depth_map_path = process_image(filepath)
        depth_filenames.append(os.path.basename(depth_map_path))

    return render_template('webpage.html', depth_filenames=depth_filenames)

def process_image(filepath):
    # Load and preprocess image
    img = Image.open(filepath)
    img_no_bg = remove(img)  # Remove background

    # Ensure image is in RGB mode
    if img_no_bg.mode != "RGB":
        img_no_bg = img_no_bg.convert("RGB")
    
    img_tensor = transform(img_no_bg).unsqueeze(0)

    # Generate depth map
    with torch.no_grad():
        depth = model(img_tensor)
        depth = depth.squeeze().cpu().numpy()
    
    # Normalize and save depth map
    depth_norm = cv2.normalize(depth, None, 0, 255, cv2.NORM_MINMAX).astype(np.uint8)
    depth_filepath = os.path.join(DEPTH_FOLDER, f'depth_{os.path.basename(filepath)}')
    cv2.imwrite(depth_filepath, depth_norm)

    return depth_filepath

@app.route('/depth_maps/<filename>')
def depth_image(filename):
    return send_from_directory(DEPTH_FOLDER, filename)

@app.route('/create_mesh', methods=['POST'])
def create_mesh():
    depth_files = os.listdir(DEPTH_FOLDER)
    if not depth_files:
        return "No depth maps available for mesh creation."

    combined_points = []
    max_height, max_width = 0, 0  # To store the dimensions of the depth maps

    # Loop through all depth files
    for filename in depth_files:
        depth_map_path = os.path.join(DEPTH_FOLDER, filename)
        depth_map = cv2.imread(depth_map_path, cv2.IMREAD_UNCHANGED)
        if depth_map is None:
            continue  # Skip if depth map cannot be read
        
        height, width = depth_map.shape
        max_height = max(max_height, height)
        max_width = max(max_width, width)

        # Generate point cloud from the depth map
        for y in range(height):
            for x in range(width):
                z_value = depth_map[y, x]
                if z_value > 0:  # Only include valid depth values
                    # Append (x, y, z_value) to combined_points
                    combined_points.append((x, y, z_value))

    # Convert list to numpy array
    combined_points = np.array(combined_points)

    if len(combined_points) < 4:
        return "Not enough points to create a mesh."

    # Create faces using quads (squares)
    faces = []
    for y in range(max_height - 1):
        for x in range(max_width - 1):
            # Calculate the indices for the square
            index1 = y * max_width + x
            index2 = y * max_width + (x + 1)
            index3 = (y + 1) * max_width + (x + 1)
            index4 = (y + 1) * max_width + x

            # Ensure all indices are within bounds
            if index1 < len(combined_points) and index2 < len(combined_points) and index3 < len(combined_points) and index4 < len(combined_points):
                faces.append((index1, index2, index3, index4))

    # Create mesh from the combined point cloud and the faces
    faces = np.array(faces)
    mesh = trimesh.Trimesh(vertices=combined_points, faces=faces)

    # Generate a unique filename for the mesh
    timestamp = int(time.time())
    mesh_filename = f'output_mesh_{timestamp}.ply'
    mesh_filepath = os.path.join('.', mesh_filename)

    # Export the mesh to a file
    mesh.export(mesh_filepath)

    # Serve the newly created mesh file
    return send_from_directory('.', mesh_filename, as_attachment=True)

if __name__ == '__main__':
    app.run(debug=True)
