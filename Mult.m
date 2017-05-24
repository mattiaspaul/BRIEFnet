classdef Mult < dagnn.ElementWise
  %MULT DagNN mult layer
  %   The MULT layer takes the pointwise multiplication of all its two inputs and store the result
  %   as its only output.

  properties (Transient)
    numInputs
  end

  methods
    function outputs = forward(obj, inputs, params)
      obj.numInputs = numel(inputs) ;
      outputs{1} = inputs{1}.*inputs{2} ;
      
    end

    function [derInputs, derParams] = backward(obj, inputs, params, derOutputs)
      derInputs{1} = inputs{2}.*derOutputs{1} ;
      derInputs{2} = inputs{1}.*derOutputs{1} ;
      
      derParams = {} ;
    end

    function outputSizes = getOutputSizes(obj, inputSizes)
      outputSizes{1} = inputSizes{1} ;
      
    end

    function rfs = getReceptiveFields(obj)
      numInputs = numel(obj.net.layers(obj.layerIndex).inputs) ;
      rfs.size = [1 1] ;
      rfs.stride = [1 1] ;
      rfs.offset = [1 1] ;
      rfs = repmat(rfs, numInputs, 1) ;
    end

    function obj = Mult(varargin)
      obj.load(varargin) ;
    end
  end
end
