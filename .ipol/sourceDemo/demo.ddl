{
   "build": {
      "url": "git@github.com:kerautret/DecSiRP.git",
      "rev": "origin/main",
      "dockerfile": ".ipol/Dockerfile"
   },
   "general": {
      "requirements": "docker",
      "demo_title": "Demonstration of A Voting Scheme for Decomposing Surfaces into Round Primitives",
      "description": "<h4>A Voting Scheme for Decomposing Surfaces into Round Primitives</h4> <p>This demonstration show the algorithm to decompose surfaces into Round Primitives that is also available on tge GitHub repository: <a style='color:#0000BB' href=https://github.com/kerautret/xxx > here</a>.",
      "input_description": [
         "Click on an image or upload one to use it as the algorithm input."
      ],
      "param_description": [
         "You can choose the parameter and run the algorithm."
      ],
      "xlink_article": "https://www.ipol.im/",
      "timeout": 1600
   },
   "inputs": [
      {
         "description": "input mesh file (<b>.obj only</b>)",
         "required": true,
         "dtype": "x",
         "ext": ".obj",
         "type": "data",
         "max_pixels": "7000*7000",
         "max_weight": "50*1024*1024"
      }
   ],
   "params": [
      {
         "id": "l",
         "label": "l: the length of the segment for computing accumulation. ",
         "type": "range",
         "visible": true,
         "comments": "Set the maximal radius used to compute the accumulation.",
         "values": {
            "default": 15,
            "max": 100,
            "min": 1,
            "step": 1
         }
      },
    {
         "id": "a",
         "label": "a: minimum accumulation value to consider candidate voxel.",
         "type": "range",
         "visible": true,
         "comments": "useful to remove spurious segments or unwanted regions",
         "values": {
            "default": 1,
            "max": 100,
            "min": 1,
            "step": 1
         }
      }
   ],
   "run": "bash $bin/.ipol/run.sh input_0.obj  ${maxRadius} ${thresholdAcc} ${compAcc} ${ajustScale} ${invNormals} ",
   "results": [
      {
         "type": "html_text",
         "contents": "The resulting compressed/decompressed files  are available below and the comparison of the reconstructed meshes with the input is possible through the 3d display that allows to display the input or result meshes with the size information. Note that the performance and execution time are available on output."
      },
      {
         "type": "gallery",
         "contents": {
            "Input": {
               "img": "visuInput.png"
            },
            "Resulting max accumulation ": {
               "img": "visuAccMax.png"
            },
            "Accumulation voxels values": {
               "img": "visuAccVoxels.png",
               "visible": "info.displayAcc==1"
            }
         }
      },
      {
         "contents": "'<br>Mesh visualisation :<br>' ",
         "type": "html_text"
      },
      {
         "type": "html_text",
         "contents": "info.url"
      },
      {
         "type": "file_download",
         "label": "<h3> You can download resulting file here:</h3>",
         "contents": {
            "Input mesh": "input_0.obj",
            "Input rendering": "visuInput.png",
            "Accumulation voxels values": "visuAccVoxels.png",
            "resultAcc.obj": "resultAcc.obj"
         }
      },
      {
         "contents": "stdout.txt",
         "label": "Output",
         "type": "text_file"
      }
   ]
}
