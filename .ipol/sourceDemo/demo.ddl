{
   "build": {
      "url": "git@github.com:kerautret/DecSiRP.git",
      "rev": "origin/main",
      "dockerfile": ".ipol/Dockerfile"
   },
   "general": {
      "requirements": "docker",
      "demo_title": "Demonstration of A Voting Scheme for Decomposing Surfaces into Round Primitives",
      "description": "<h4>A Voting Scheme for Decomposing Surfaces into Round Primitives</h4> <p>This demonstration show the algorithm to decompose surfaces into Round Primitives that is also available on tge GitHub repository: <a style='color:#0000BB' href=https://github.com/Seangongx/DecSiRP > here</a>.",
      "input_description": [
         "Click on an image or upload one to use it as the algorithm input."
      ],
      "param_description": [
         "You can choose the parameter and run the algorithm. "
      ],
      "xlink_article": "https://www.ipol.im/",
      "timeout": 1600
   },
   "inputs": [
      {
         "description": "input mesh file (<b>.obj only</b>)",
         "required": false,
         "dtype": "x",
         "ext": ".obj",
         "type": "data"
      },
      {
         "description": "input volumetric file (<b>.vol only</b>)",
         "required": false,
         "dtype": "x",
         "ext": ".vol",
         "type": "data"
      }
   ],
   "params": [
      {
         "type": "label",
         "label": "Parameters related to the voting (Step 1):"
      },
      {
         "id": "l",
         "label": "l: control the size of detectable structures (max size).",
         "type": "range",
         "visible": true,
         "comments": "it controls the maximal size of detectable feature (see d parameter for min size)",
         "values": {
            "default": 15,
            "max": 100,
            "min": 1,
            "step": 1
         }
      },
      {
         "type": "label",
         "label": "Parameters related to the cluster (Step 2):"
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
      },
      {
         "id": "c",
         "label": "c: minimum confidence value to consider candidate voxel.",
         "type": "range",
         "visible": true,
         "comments": "in general 0.5, increasing value to 1 reduce candidate with perfectly round area, while lower value is less selective.",
         "values": {
            "default": 0.5,
            "max": 1,
            "min": 0,
            "step": 0.05
         }
      },
      {
         "id": "d",
         "label": "d: control the size of detectable structures (min size).",
         "type": "range",
         "visible": true,
         "comments": "it controls the minimal size of detectable feature (see l parameter for max size)",
         "values": {
            "default": 1,
            "max": 100,
            "min": 0,
            "step": 0.01
         }
      },
      {
         "type": "label",
         "label": "Parameters related to label extension (Step 3):"
      },
      {
         "id": "lambda",
         "label": "λ: Controls label extension.",
         "type": "range",
         "visible": true,
         "comments": " values close to 1 limit extension, while values close to 0 allow maximal extension.",
         "values": {
            "default": 0.5,
            "max": 1,
            "min": 0,
            "step": 0.05
         }
      },
      {
         "id": "rho",
         "label": "ρ: probability threshold for the label extension process.",
         "type": "range",
         "visible": true,
         "comments": "typically set to 10<sup>−3</sup>",
         "values": {
            "default": 0.001,
            "max": 1,
            "min": 0,
            "step": 0.05
         }
      }
   ],
   "run": "bash $bin/.ipol/run.sh input_0.obj input_1.vol  ${l} ${a} ${c} ${d} ${lambda} ${rho} ",
   "results": [
      {
         "type": "gallery",
         "contents": {
            "Input": {
               "img": "visuInput.png"
            },
            "Output": {
               "img": "visuResult.png"
            },
            "Interface": {
               "img": "visuInterface.png"
            }
         }
      },
      {
         "contents": "resultSeg_mergeMat.obj",
         "type": "three_d",
         "label": "3D output file"
      },
      {
         "type": "file_download",
         "label": "<h3> You can download resulting file here:</h3>",
         "contents": {
            "Input mesh": "input_0.obj",
            "Result segmentation (merge obj, mtl)": "resultSeg_mergeMat.obj",
            "Result segmentation (obj)": "resultSeg.obj",
            "Result segmentation (mtl)": "resultSeg.mtl"
         }
      },
      {
         "contents": "stdout.txt",
         "label": "Output",
         "type": "text_file"
      }
   ],
   "archive": {
      "enable_reconstruct": true,
      "files": {
         "input_0.obj": "Input image",
         "resultSeg_mergeMat.obj": "mesh result",
         "visuInput.png": "Input rendering",
         "visuResult.png": "Result of segmentation ",
         "visuInterface.png": "Interface visualisation",
         "resultAcc.obj": "resultAcc.obj",
         "stdout.txt": "Output"
      },
      "params": [
         "l",
         "a",
         "c",
         "d",
         "lambda",
         "rho"
      ],
      "info": {
         "run_time": "run time"
      }
   }
}
