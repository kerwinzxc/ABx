function getDamage(baseMin, baseMax, critical)
  if (critical == true) then
    return baseMax
  end
  return ((baseMax - baseMin) * math.random()) + baseMin
end
