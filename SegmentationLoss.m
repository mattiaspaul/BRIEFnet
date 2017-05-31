classdef SegmentationLoss < dagnn.Loss

  methods
    function outputs = forward(obj, inputs, params)
        weight2=(sum(sum(inputs{2}-1,2),1)+500)./(123*84);
        weight1=repmat(1-weight2,123,84,1,1);
        mass=repmat(weight2,123,84,1,1)*10;
        mass(inputs{2}==2)=weight1(inputs{2}==2)*10;
        %edit 22-2-17 *10->*40 +500->+50 

      %mass = single(inputs{2}==2).*0.5+0.5;%sum(sum(inputs{2} > 0,2),1) + 1 ;
      outputs{1} = vl_nnloss(inputs{1}, inputs{2}, [], ...
                             'loss', obj.loss, ...
                             'instanceWeights', mass) ;
      n = obj.numAveraged ;
      m = n + size(inputs{1},4) ;
      obj.average = (n * obj.average + double(gather(outputs{1}))) / m ;
      obj.numAveraged = m ;
    end

    function [derInputs, derParams] = backward(obj, inputs, params, derOutputs)
        weight2=(sum(sum(inputs{2}-1,2),1)+500)./(123*84);
        weight1=repmat(1-weight2,123,84,1,1);
        mass=repmat(weight2,123,84,1,1)*10;
        mass(inputs{2}==2)=weight1(inputs{2}==2)*10;

        %mass = single(inputs{2}==2).*0.5+0.5;%sum(sum(inputs{2} > 0,2),1) + 1 ;
      derInputs{1} = vl_nnloss(inputs{1}, inputs{2}, derOutputs{1}, ...
                               'loss', obj.loss, ...
                               'instanceWeights', mass) ;
      derInputs{2} = [] ;
      derParams = {} ;
    end

    function obj = SegmentationLoss(varargin)
      obj.load(varargin) ;
    end
  end
end
