# nobodyDL——小人物深度学习
    cudaBase.cpp    CUDA基本操作
    learnForest.cpp 是几年前写的，现不再维护，有需要时重构
    nnet.h  神经网络头文件
    nnetBase.cpp  神经网络配置解析
    nnetConvolution.cpp 神经网络卷积层
    nnetModel.cpp 神经网络训练+预测
    optimization.h  优化算法头文件
    optimLBFGS.cpp  优化算法LBFGS
    optimSearch.cpp 优化算法步长搜索
    optimVSGD.cpp   优化算法SGD
    sparse.h  稀疏矩阵头文件
    tensor.h  张量头文件
    tensorVML.cpp   张量向量计算
    xpu.h   设备头文件
    
    
    imagenet224_hash_15.cfg  15层全卷积网络结构，256 bits 哈希学习
        训练30轮准确率68.1%（single model single crop），相当于VGGNet 16层网络结构（4096 bits）的准确率
        用GTX980ti单卡训练112小时，合每秒95帧
        
    两卡训练加速1.8（最小的模型）~1.9+倍，测试发现对于并行加速，IO和带宽影响各占一半
