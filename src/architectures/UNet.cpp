#include "UNet.h"

UNetImpl::UNetImpl(int _num_classes, std::string encoder_name, std::string pretrained_path, int encoder_depth,
                   std::vector<int> decoder_channels, bool use_attention){
    num_classes = _num_classes;
	auto encoder_param = encoder_params();
	std::vector<int> encoder_channels = encoder_param[encoder_name]["out_channels"];
	if (!encoder_param.contains(encoder_name))
		throw "encoder name must in {resnet18, resnet34, resnet50, resnet101, resnet150, \
				resnext50_32x4d, resnext101_32x8d, vgg11, vgg11_bn, vgg13, vgg13_bn, \
				vgg16, vgg16_bn, vgg19, vgg19_bn,}";
	if (encoder_param[encoder_name]["class_type"] == "resnet")
		encoder = new ResNetImpl(encoder_param[encoder_name]["layers"], 1000, encoder_name);
	else if (encoder_param[encoder_name]["class_type"] == "vgg")
		encoder = new VGGImpl(encoder_param[encoder_name]["cfg"], 1000, encoder_param[encoder_name]["batch_norm"]);
	else throw "unknown error in backbone initialization";

	encoder->load_pretrained(pretrained_path);
    decoder = UNetDecoder(encoder_channels,decoder_channels, encoder_depth, use_attention, false);
    segmentation_head = SegmentationHead(decoder_channels[decoder_channels.size()-1], num_classes, 1, 1);

    register_module("encoder",std::shared_ptr<Backbone>(encoder));
    register_module("decoder",decoder);
    register_module("segmentation_head",segmentation_head);
}

torch::Tensor UNetImpl::forward(torch::Tensor x){
    std::vector<torch::Tensor> features = encoder->features(x);
    x = decoder->forward(features);
    x = segmentation_head->forward(x);
    return x;
}
