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
         "id": "lambda",
         "label": "λ: to govern the label extension process (from probabilty expansion value).",
         "type": "range",
         "visible": true,
         "comments": "typically set to 10−3",
         "values": {
            "default": 0.001,
            "max": 1,
            "min": 0,
            "step": 0.05
         }
      },
        {
         "id": "rho",
         "label": "ρ: to govern the label extension process (behaviour).",
         "type": "range",
         "visible": true,
         "comments": "1 means no extension, and the closer ρ is to 0, the greater the extension",
         "values": {
            "default": 0.5,
            "max": 1,
            "min": 0,
            "step": 0.05
         }
      }
     
   ],
   "run": "bash $bin/.ipol/run.sh input_0.obj  ${l} ${a} ${c} ${d} ${lambda} ${rho} ",
   "results": [
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
   ]
}
