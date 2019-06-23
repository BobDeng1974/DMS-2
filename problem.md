Dear Shubha,

My command is like this:

python3 /home/yang/intel/openvino/deployment_tools/model_optimizer/mo_tf.py \
   --input_model \
/home/share/models/research/object_detection/train/export_dsm_focal/frozen_inference_graph.pb \
  --tensorflow_use_custom_operations_config /home/yang/intel/openvino/depl    oyment_tools/model_optimizer/extensions/front/tf/ssd_v2_support.json \
  --tensorflow_object_detection_api_pipeline_config /home/share/models/res    earch/object_detection/train/export_dsm_focal/pipeline.config
Everything is OK when i use the .pb file obtaining by TF Object Detection API(TF==1.5.0 py2.7).

When i try to convert the .pd file obtaining by TF Object Detection API(TF==1.12.0 py2.7),the above error occured.

Does Model Optimizer have a limit to the version of TF Object Detection API?

Thanks,

Jinyang
